# Halloc : Algorithme d'allocation mémoire (TCHALLOC) V1.1.0

Environnement : Pour Linux 

Fonctionnement :
# I - Gestion de la mémoire avec brk / sbrk (moremem.c / moremem.h)
L'algorithme d'allocation mémoire halloc est tout d'abord basé sur un segment de la mémoire extensible, que l'on appelera par la suite HEAP.  
Ce segment en mémoire n'est pas présent au début du programme, puisqu'il est inutile de créer si il n'y-a aucune allocation mémoire à faire. Il est possible de le créer avec la fonction `moremem.h/mmem_createMemorySpace` qui va créer l'espace mémoire à l'aide de l'appel système `sbrk(0)`, celà créera un espace de taille nulle. À noter que par la suite, c'est lors de la première allocation que cet espace est créé.  
Ce segment mémoire peut par la suite être étendu une fois créé, ce qui est fondamental pour le fonctionnement de l'algorithme. Pour se faire, nous utiliserons la fonction `moremem.h/mmem_extend`, qui l'étendera d'une certaine taille (en bytes) ajoutée, à l'aide de l'appel système `brk`.  
Enfin, nous pouvons de même détruire ce segment mémoire une fois qu'il n'est plus utilisé avec la fonction `moremem.h/mmem_uncreate` à l'aide l'appel système `brk`.

# II - Les chunks (heap.h)
Tout d'abord, l'algorithme utilise un système de chunks. Il s'agit de blocks dans le segment HEAP ayant une certaine taille, qui possèdent une partie metadata et une partie data. La metadata contient toutes les informations à propos de celui-ci, comme sa taille, la taille du précédent, si il est libre ou pas, ainsi que des pointeurs, l'un vers le suivant de la binlist et le précédent. 
```
Chunk A
| size <--- Taille (metadata + data)                         ]
| prev_size <--- Taille du précédent (ici il n'y-en a pas)   ]
| flags <--- Si il est utilisé, si le précédent est utilisé  ] <--- Metadata
| fd <--- Pointeur vers le prochain de la binlist            ]
| bk <--- Pointeur vers le précédent de la binlist           ]
| [----------------] <-- Data
Chunk B
| size
| prev_size <--- Taille De Chunk A
| flags
| fd
| bk
| [----------------] <-- Data
...

```
On dira d'un chunk qu'il est libre si il n'est simplement plus utilisé, si il est en quelque sortes "brouillon" dans la mémoire, et en particulier que l'on peut l'utiliser pour l'écraser en faveur d'autres allocations.

La deuxième partie du chunk, étant juste après la metadata dans la mémoire, est la data, qui contient les données du chunk, c'est à dire, lorsque nous allouons avec `halloc`, la fonction renverra un pointeur vers cette partie, et donc l'utilisateur de l'algorithme l'utilisera pour son usage. 

# III - La binlist (heap.c)
Une fois libéré, on insérera le chunk dans la binlist, il s'agit d'une double linked list, qui contient chaque chunk libéré, chaque libéré pointe vers le suivant (`NULL` si dernier) et vers le précédent (`NULL` si premier) : 
Voici un exemple de configuration :
```
CHUNK A (libéré)
| ...
| fd = CHUNK C
| bk = CHUNK D
| ...
CHUNK B (utilisé)
| ...
CHUNK C (libéré)
| ...
| fd = NULL <-- dernier
| bk = CHUNK A
| ...
CHUNK D (libéré)
| ...
| fd = CHUNK A
| bk = NULL <-- premier
| ...
CHUNK E (utilisé)
| ...
```
Dans cet exemple ci-dessus, le `chunk D` est le premier de la binlist, puis le `chunk A` et enfin le `chunk C`. Étant donné que chacun pointe vers le suivant (`NULL` si dernier) et vers le précédent (`NULL` si premier), il est facile de les parcourir et de rapidement savoir quels chunks sont libres à l'aide des fonctions `heap.c/first_bin_list`, qui renvoie le premier élément de la binlist, `heap.c/bin_list_last`, qui renvoie le dernier, `heap.c/follow_fd` (préprocesseur), qui renvoie le prochain à partir d'un certain chunk de la binlist et `heap.c/empty_bin_list`, qui retourne si la binlist est vide.

# IV - La recherche de zone libre 
La fonction `heap.c/find_and_reserve_space` permet de trouver une zone mémoire convenable pour un chunk d'une certaine taille.
Il y'a deux options, soit, assez de place est disponible dans les chunks de la binlist, ou soit, il n'y-en a pas (ou la binlist est vide), dans le cas échéant, le heap est étendu de la taille du chunk souhaité à l'aide de `heap.c/extendHeap`, qui étend le heap graĉe à `moremem.h/mmem_extend`.
Pour chercher de la place dans la binlist, la fonction fait appel à `heap.c/find_and_reserve_space_in_bins`. Le déroulement de cette dernière est le suivant : Pour chacun des chunks de la binlist, en commençant par le premier (à l'aide de `heap.c/first_bin_list`), on compare sa taille avec la taille que nous souhaitons allouer. Si il y'a exactement assez de place, alors on alloue le chunk à la place de celui qui vient d'être trouvé, si y'a plus de place que nécessaire, nous séparerons le chunk trouvé en deux parties, à l'aide de `heap.c/split_chunk` celle qui sera libre (donc la place en trop) et celle où le nouveau chunk sera alloué. Si aucun chunk de la binlist ne possède assez de place, il s'agit du cas échéant, et le heap est étendu comme mentionné plus tôt.

# V - L'allocation (halloc) (heap.c / heap.h)
La fonction `heap.h/heap_allocate` crée un nouveau chunk telle que la data aie une certaine taille et renvoie un pointeur vers celle-ci. Tout d'abord, si le heap n'est pas initialisé (crée), la fonction appelera la fonction `heap.c/initHeap`, qui elle-même appelle `moremem.h/mmem_createMemorySpace` pour créer le segment mémoire heap. Après celà, la fonction appelera la fonction `heap.c/find_and_reserve_space` pour trouver une zone mémoire libre, la plus optimale possible. Une fois trouvée, si nous sommes en train d'allouer le chunk dans un espace qui contenait déjà un (ancien) chunk libre, alors nous effectuons un `unlink` sur celui-ci, et nous indiquons au prochain chunk, qui suivait donc cet ancien chunk qu'il est désormais utilisé :
```
Chunk A
...
Chunk n <--- Chunk libre dans lequel on allouera le nouveau chunk
| ... 
Chunk n+1
| prev_used = 1 <--- On indique que le chunk précédent (n) est utilisé
```
Sinon, le heap est étendu de la taille nécéssaire et nous créeons le nouveau chunk à la fin de ce dernier.
```
Chunk A
| ...
Chunk B
| ...
(...)
---------------
|           <---- Nouvel espace
| Chunk nouveau
| |
| |
```
Le nouveau chunk est ensuite créé et la fonction renvoie un pointeur vers la data de celui-ci qui a la taille souhaitée.

# VI - La libération de mémoire (hfree) (heap.c / heap.h)
Pour libérer un chunk, nous utiliserons la fonction `heap.h/heap_free`, qui libère un chunk alloué, pour se faire, nous pourrions simplement indiquer qu'il n'est plus utilisé et le rajouter à la binlist, mais dans cet algorithme, des optimisations sont faites, qui sont très importantes.

## Optimisation à gauche
Si le chunk précédent est libre, alors nous allons fusionner le chunk que nous voulons libérer avec celui-ci, pour celà, il suffit simplement de mettre à jour la taille du chunk libre et de l'étendre à la taille du chunk à libérer :
```
...
Chunk n-1 libre
| used = 0
| taille = x
Chunk n à libérer
| ...
```
Devient :
```
...
Chunk n-1 libre
| used = 0
| taille = x + taille(chunk n)
Chunk n à libérer (inexistant à présent, car "consommé" par le chunk n-1)
```
Nous nous aussi devons de mettre à jour les informations sur celui-ci du prochain chunk (si il n'est pas dernier).

## Pas d'optimisation à gauche
Sinon, dans le cas où le précédent est utilisé, nous libérons bien ce chunk d'une manière "standard", nous l'ajoutons à la binlist, nous indiquons qu'il n'est plus utilisé et nous mettons à jour les informations sur ce chunk pour le chunk suivant.

## Optimisation à droite
De même, si le chunk à libérer n'est pas le dernier, nous regardons si le prochain chunk n'est pas utilisé auquel-cas nous fusionnerons ce chunk avec le suivant :
```
...
Chunk n à libérer
| taille = x
Chunk n + 1 libre
| used  = 0
```
Devient :
```
...
Chunk n à libérer
| taille = x + taille(chunk n+1)
Chunk n + 1 inexistant par "consommé" par le chunk n
```
Ensuite, nous effectuerons un `unlink` (retirer de la binlist) sur le chunk n + 1 (suivant au chunk à libérer), et comme d'habitude, nous mettons à jour les informations du prochain chunk.
