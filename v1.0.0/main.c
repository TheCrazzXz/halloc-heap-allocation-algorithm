/*
    password manager
*/

#include "heap/heap.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 100
#define USER_NO_USER 999

typedef enum ps_manager_user_privilege
{
    PRIVILEGE_USER_NONE,
    PRIVILEGE_USER_USER,
    PRIVILEGE_USER_ADMIN,
    PRIVILE_USER_ROOT,
} user_privilege;


typedef struct ps_manager_user
{
    char* name;
    char* password;
    int used;
    int id;
    user_privilege privilegeLevel;
} user;

typedef struct ps_manager_user_list
{  
    user users[MAX_USERS];
    ulong length;
} user_list;

#define createUserList() { {}, 0 }
#define check_if_user_connected() if(currentUser == USER_NO_USER) {printf("Error : No user selected\n"); continue;}; 
ulong currentUser = USER_NO_USER;

void std_userinput(char* dst)
{
    char c;
    while(1)
    {
        c = getc(stdin);
        if(c == '\n')
        {
            *dst++ = (char)0;
            return;
        }
        *dst++ = c;
    }
   
}

ulong serach_for_deleted_user(user_list* users)
{
    for(ulong i = 0 ; i < users->length ; i++)
    {
        if(users->users[i].used == 0)
        {
            return i;
        }
    }
    return USER_NO_USER;        
}

void add_user(user_list* users, user* user)
{
    ulong index;

    ulong search = serach_for_deleted_user(users);
    if(search < MAX_USERS)
    {
        index = search;
    }
    else
    {
        if(users->length == MAX_USERS)
        {
            printf("Error : Too many users");
            return;
        }
        else
        {
            index = users->length;
        }
    }

    

    users->users[index] = *user;

    users->length++;
}
void delete_user(user_list* users, ulong index)
{
    hfree(users->users[index].name);
    hfree(users->users[index].password);
    users->users[index].used = 0;
}

int user_exists(user_list* users, char* name)
{
    for(ulong i = 0 ; i < users->length ; i++ )
    {
        if(strcmp(users->users[i].name, name) == 0 && users->users[i].used == 1)
        {
            return 1;
        }
    }
    return 0;
}

void delete_all_users(user_list* users)
{
    for(ulong i = 0 ; i < users->length ; i++)
    {
        if(users->users[i].used == 1)
        {
            delete_user(users, i);
        }
    }
}

void prompt_new_user(user_list* users)
{
    user new_user;
    char* name;
    char* password;

    name = halloc(64);
    password = halloc(64);

    new_user.used = 1;
    new_user.id = rand();

    printf("Enter username : ");
    std_userinput(name);

    printf("Enter password : ");
    std_userinput(password);

    if(user_exists(users, name))
    {
        printf("Error : User with name : %s does already exist\n", name);
        return;
    }

    new_user.name = name;
    new_user.password = password;
    new_user.privilegeLevel = PRIVILEGE_USER_USER;

    add_user(users, &new_user);    

    currentUser = users->length - 1; 
}

void print_users(user_list* users)
{
    printf("[USERS] : \n");
    for(ulong i = 0 ; i < users->length ; i++)
    {
        if(users->users[i].used == 1)
        {
            printf("[%lu] : {\n\tname : '%s'\n\tid : %d\n}\n", i, users->users[i].name, users->users[i].id);
        }
    }
}

void prompt_modify_current_user(user_list* users)
{
    user* modifiy_user;
    char* name;
    int modifyPassword;
    name = halloc(64);

    modifiy_user = &users->users[currentUser];


    printf("Enter username (previous : %s) : ", modifiy_user->name);
    std_userinput(name);

    if(user_exists(users, name))
    {
        printf("Error : User with name : %s does already exist\n", name);
        return;
    }
    hfree(modifiy_user->name);
    modifiy_user->name = name;

    printf("Modify password ? [0 : No | 1 or anything else : Yes]");
    scanf("%d", &modifyPassword); 
    fgetc(stdin);

    if(modifyPassword != 0)
    {
        printf("Enter password (previous : %s): ", modifiy_user->password);
        std_userinput(modifiy_user->password);
    }

   

}
void whoami(user_list* users)
{
    printf("name : '%s'\tid : %d\n", users->users[currentUser].name, users->users[currentUser].id);
}
typedef enum login_user_return
{
    USER_NOT_FOUND,
    USER_WRONG_PASSWORD,
    USER_CONNECTED
} login_user_return;

login_user_return login_user(user_list* users, char* name, char* password)
{
    for(ulong i = 0 ; i < users->length ; i++ )
    {
        if(users->users[i].used == 1)
        {
            if(strcmp(users->users[i].name, name) == 0)
            {
                if(strcmp(users->users[i].password, password) == 0)
                {
                    currentUser = i;
                    return USER_CONNECTED;
                }
                else
                {
                    return USER_WRONG_PASSWORD;
                }
                
            }
        }
        
    }
    return USER_NOT_FOUND;
}

void prompt_login_user(user_list* users)
{
    char* name;
    char* password;

    name = halloc(64);
    password = halloc(64);

    printf("Enter username : ");
    std_userinput(name);
    printf("Enter password for user '%s' : ", name);
    std_userinput(password);

    int ret = login_user(users, name, password);

    if(ret == USER_CONNECTED)
    {
        printf("Authentification success\n");
    }
    else if(ret == USER_NOT_FOUND)
    {
        printf("Error : User not found\n");
    }
    else if(ret == USER_WRONG_PASSWORD)
    {
        printf("Authentification failure\n");
    }
    else
    {
        printf("NOT SUPPOSED TO BE HERE\n");
    }

    hfree(name);
    hfree(password);
}

void createAdminUser(user_list* users)
{
    user admin;

    admin.privilegeLevel = PRIVILEGE_USER_ADMIN;
    admin.id = 0;
    admin.used = 1;
    admin.name = halloc(64);
    admin.password = halloc(64);

    strcpy(admin.name, "I'm admin");
    strcpy(admin.password, " ");

    add_user(users, &admin);    
}

void print_actions()
{
    printf("List of all actions : \n1) Register as new user\n2) Login user\n3) Modify user\n4) Whoami\n5) Show all users\n6) Delete user\n7) Disconnect\n8) exit\n\n");
}

int main(void)
{
    int action;
    srand(time(NULL));
    user_list users = createUserList();
    memfill(users.users, 0, sizeof(user) * MAX_USERS);

    createAdminUser(&users);

    printf("\\\\|Secured User manager 3000|////\n");

    while(1)
    {
        print_actions();
        printf("Enter action : \n");
        scanf("%d", &action);
        fgetc(stdin);
        if(action == 1)
        {
            prompt_new_user(&users);
        }
        // 4 - 1 - 5 - 1 - 6 - 2
        else if(action == 2)
        {
            prompt_login_user(&users);
        }
        else if(action == 3)
        {
            check_if_user_connected();
            prompt_modify_current_user(&users);
        }
        else if(action == 4)
        {
            check_if_user_connected();
            whoami(&users);
        }
        else if(action == 5)
        {
            check_if_user_connected();
            print_users(&users);
        }
        else if(action == 6)
        {
            check_if_user_connected();
            delete_user(&users, currentUser);
            printf("Deleted user\n");
            currentUser = USER_NO_USER;
        }
        else if(action == 7)
        {
            currentUser = USER_NO_USER;
        }
        else if(action == 8)
        {
            break;
        }
        else
        {
            printf("Invalid action\n");
            continue;
        }
        #if DEBUG_MODE_ENABLED >= 1
            print_heap(0);
            print_bin_list();
        #endif
    }

    delete_all_users(&users);

    hcleanup();


}