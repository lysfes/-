#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "models.h"
#include "user_store_csv.h"

static void trim_newline(char *s){ 
    if(!s) return;
    size_t n=strlen(s);
    while(n>0 && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n]=0;
}

static int read_line(const char *prompt, char *out, size_t cap){
    printf("%s", prompt);
    fflush(stdout);
    if(!fgets(out, (int)cap, stdin)) return 0;
    trim_newline(out);
    return 1;
}

static void press_enter(void){
    printf("\nPress Enter to continue...");
    fflush(stdout);
    char buf[8]; fgets(buf, sizeof(buf), stdin);
}

// ------ Menu declarations ------
static int  loginMenu(void);
static void managerMenu(void);
static void customerMenu(const char *username);
static void register_flow(void);

// ------ Main ------
int main(void){
    if (loadData()!=0){
        fprintf(stderr, "Failed to read customers.csv (starting with empty store).\n");
    }

    for(;;){
        int role = loginMenu();
        if (role == 0) break;

        if (role == 1){
            // Manager password (can be hard-coded for coursework)
            char pw[64];
            read_line("Enter manager password: ", pw, sizeof(pw));
            if (strcmp(pw,"123456")!=0){
                puts("Wrong password!");
                continue;
            }
            managerMenu();
        } else if (role == 2){
            printf("\n=== Customer Login ===\n");
            char uname[30];
            if(!read_line("Username: ", uname, sizeof(uname))) continue;
            Customer c;
            if (getCustomerByName(uname, &c)){
                customerMenu(uname);
            } else {
                printf("User not found. Register? (y/n): ");
                char b[8]; fgets(b,sizeof(b),stdin);
                if(b[0]=='y' || b[0]=='Y'){
                    // Registration flow
                    strncpy(uname, uname, sizeof(uname)-1);
                    register_flow();
                }
            }
        }
    }

    saveData();
    puts("Exited.");
    return 0;
}

// ------ Login menu ------
static int loginMenu(void){
    for(;;){
        printf("\n=== Login Menu ===\n");
        printf("1) Manager login\n");
        printf("2) Customer login\n");
        printf("0) Exit program\n");
        printf("Choice: ");
        char buf[16];
        if(!fgets(buf,sizeof(buf),stdin)) return 0;
        int c = atoi(buf);
        if (c==0 || c==1 || c==2) return c;
        puts("Invalid choice.");
    }
}

// ------ Customer registration (calls addCustomer, initial balance=0) ------
static void register_flow(void){
    char u[30], p[20];
    printf("\n=== Customer Registration ===\n");
    if(!read_line("Set username (letters/digits/_/- only): ", u, sizeof(u))) return;
    if(!read_line("Phone number (digits/+ only): ",           p, sizeof(p))) return;
    int rc = addCustomer(u, p, 0.0f, ACTOR_CUSTOMER);
    if (rc==0) puts("Registration successful.");
    else if (rc==-2) puts("Registration failed: username already exists.");
    else if (rc==-3) puts("Registration failed: user limit reached.");
    else puts("Registration failed: invalid input.");
    press_enter();
}

// ------ Manager menu and operations ------
static void list_customers_ui(void){
    Customer arr[256];
    int n = listCustomers(arr, 256);
    printf("\n%-20s %-16s %-10s %-10s\n", "Username", "Phone", "Balance (CNY)", "OrderCount");
    printf("-------------------------------------------------------------\n");
    for(int i=0;i<n;i++){
        printf("%-20s %-16s %-10.2f %-10d\n",
               arr[i].username, arr[i].phone, arr[i].balance, arr[i].orderCount);
    }
}

static void manager_add_user_ui(void){
    char u[30], p[20], mbuf[32];
    if(!read_line("New username: ", u, sizeof(u))) return;
    if(!read_line("Phone number: ",    p, sizeof(p))) return;
    if(!read_line("Initial balance: ", mbuf, sizeof(mbuf))) return;
    float initBal = (float)atof(mbuf);
    int rc = addCustomer(u, p, initBal, ACTOR_MANAGER);
    if (rc==0) puts("Add successful.");
    else if (rc==-2) puts("Failed: username already exists.");
    else if (rc==-3) puts("Failed: capacity full.");
    else puts("Failed: invalid input.");
}

static void manager_edit_user_ui(void){
    char u[30], p[20], mbuf[32];
    if(!read_line("Username to modify: ", u, sizeof(u))) return;
    if(!read_line("New phone (Enter to keep): ", p, sizeof(p))) return;
    if(!read_line("New balance: ", mbuf, sizeof(mbuf))) return;
    float bal = (float)atof(mbuf);
    const char *np = (p[0]? p : NULL);
    int rc = editCustomer(u, np, bal);
    if (rc==0) puts("Modify successful.");
    else if (rc==-1) puts("Modify failed: user not found.");
    else if (rc==-2) puts("Modify failed: invalid phone.");
    else if (rc==-3) puts("Modify failed: balance cannot be negative.");
    else puts("Modify failed.");
}

static void manager_remove_user_ui(void){
    char u[30];
    if(!read_line("Username to delete: ", u, sizeof(u))) return;
    int rc = removeCustomer(u);
    if (rc==0) puts("Delete successful.");
    else puts("Delete failed: user not found.");
}

static void managerMenu(void){
    for(;;){
        printf("\n=== Manager Menu ===\n");
        printf("1) List all users\n");
        printf("2) Add user\n");
        printf("3) Modify user (phone/balance)\n");
        printf("4) Delete user\n");
        printf("0) Return to login menu\n");
        printf("Choice: ");
        char buf[16]; if(!fgets(buf,sizeof(buf),stdin)) return;
        int c = atoi(buf);
        if (c==0) return;
        switch(c){
            case 1: list_customers_ui(); break;
            case 2: manager_add_user_ui(); break;
            case 3: manager_edit_user_ui(); break;
            case 4: manager_remove_user_ui(); break;
            default: puts("Invalid choice."); break;
        }
        press_enter();
    }
}

// ------ Customer menu (read-only) ------
static void customerMenu(const char *username){
    for(;;){
        Customer c;
        if (!getCustomerByName(username, &c)){
            puts("User not found (may have been deleted).");
            return;
        }
        printf("\n=== Customer Center (%s) ===\n", username);
        printf("Phone: %s\nBalance: %.2f\nOrder Count: %d\n",
               c.phone, c.balance, c.orderCount);

        printf("\n1) View info\n");
        printf("0) Return to login menu\n");
        printf("Choice: ");
        char buf[16]; if(!fgets(buf,sizeof(buf),stdin)) return;
        int sel = atoi(buf);
        if(sel==0) return;
        if(sel!=1) puts("Invalid choice.");
        press_enter();
    }
}
