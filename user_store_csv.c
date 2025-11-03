// user_store_csv.c
#include "user_store_csv.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_CUSTOMERS 100
static Customer g_customers[MAX_CUSTOMERS];
static int      g_customerCount = 0;

static const char *CSV_PATH = "customers.csv";

// —— 工具：去行尾换行 ——
static void trim_newline(char *s){
    if(!s) return;
    size_t n = strlen(s);
    while (n>0 && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n] = 0;
}

// —— 校验：统一限制，避免 CSV 解析纠纷（逗号分隔，禁止逗号） ——
static int username_ok(const char *u){
    if(!u || !u[0]) return 0;
    for (const char *p=u; *p; ++p){
        if (*p==',' ) return 0; // 禁止逗号，避免破坏 CSV
        if (!(isalnum((unsigned char)*p) || *p=='_' || *p=='-')) return 0;
    }
    return 1;
}
static int phone_ok(const char *p){
    if(!p || !p[0]) return 0;
    for (const char *q=p; *q; ++q){
        if (*q==',' ) return 0; // 禁止逗号
        if (!isdigit((unsigned char)*q) && *q!='+') return 0;
    }
    return 1;
}

static int find_index(const char *username){
    for(int i=0;i<g_customerCount;i++){
        if (strcmp(g_customers[i].username, username)==0) return i;
    }
    return -1;
}

// —— CSV 读写 ——
// 格式（含表头）：username,phone,balance,orderCount
static void ensure_csv_exists(void){
    FILE *fp = fopen(CSV_PATH, "r");
    if (fp){ fclose(fp); return; }
    fp = fopen(CSV_PATH, "w");
    if (fp){
        fprintf(fp, "username,phone,balance,orderCount\n");
        fclose(fp);
    }
}

int loadData(void){
    g_customerCount = 0;
    ensure_csv_exists();

    FILE *fp = fopen(CSV_PATH, "r");
    if (!fp) return -1;

    char line[512];
    int is_first = 1;
    while (fgets(line, sizeof(line), fp)){
        trim_newline(line);
        if (is_first){
            is_first = 0;
            // 跳过表头
            if (strncmp(line, "username,", 9)==0) continue;
        }
        if (line[0]==0) continue;

        // 解析：最多 4 段
        // 注意：用 %[^,] 捕获到逗号之前的所有字符
        char uname[30]={0}, phone[20]={0};
        float bal=0.0f; int oc=0;
        int matched = sscanf(line, " %29[^,],%19[^,],%f,%d", uname, phone, &bal, &oc);
        if (matched == 4){
            if (g_customerCount < MAX_CUSTOMERS){
                Customer c; memset(&c, 0, sizeof(c));
                strncpy(c.username, uname, sizeof(c.username)-1);
                strncpy(c.phone,    phone, sizeof(c.phone)-1);
                c.balance    = bal;
                c.orderCount = oc;
                g_customers[g_customerCount++] = c;
            }
        }
    }
    fclose(fp);
    return 0;
}

void saveData(void){
    // 原子写：写 .tmp，再 rename 覆盖
    char tmpPath[256]; snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", CSV_PATH);
    FILE *fp = fopen(tmpPath, "w");
    if (!fp){
        fprintf(stderr, "保存失败：无法写入临时文件。\n");
        return;
    }
    // 表头
    fprintf(fp, "username,phone,balance,orderCount\n");
    for(int i=0;i<g_customerCount;i++){
        const Customer *c = &g_customers[i];
        fprintf(fp, "%s,%s,%.2f,%d\n", c->username, c->phone, c->balance, c->orderCount);
    }
    fclose(fp);
    // 覆盖原文件
    remove(CSV_PATH);            // 忽略失败
    if (rename(tmpPath, CSV_PATH)!=0){
        fprintf(stderr, "保存失败：无法覆盖 %s。\n", CSV_PATH);
    }
}

// —— 读接口 ——
int listCustomers(Customer *out, int maxn){
    if (!out || maxn<=0) return 0;
    int n = (g_customerCount < maxn)? g_customerCount : maxn;
    for(int i=0;i<n;i++) out[i] = g_customers[i];
    return n;
}
int getCustomerByName(const char *username, Customer *out){
    int k = find_index(username);
    if (k<0) return 0;
    if (out) *out = g_customers[k];
    return 1;
}

// —— 写接口 ——
// 顾客注册与店长新增共用（actor 控制初始余额）
int addCustomer(const char *username, const char *phone, float initialBalance, Actor actor){
    if (g_customerCount >= MAX_CUSTOMERS) return -3;          // 满了
    if (!username_ok(username) || !phone_ok(phone)) return -1;// 非法
    if (find_index(username) >= 0) return -2;                 // 重名

    Customer c; memset(&c, 0, sizeof(c));
    strncpy(c.username, username, sizeof(c.username)-1);
    strncpy(c.phone,    phone,    sizeof(c.phone)-1);
    c.balance    = (actor==ACTOR_MANAGER)? (initialBalance < 0 ? 0.0f : initialBalance) : 0.0f;
    c.orderCount = 0;

    g_customers[g_customerCount++] = c;
    return 0;
}

int removeCustomer(const char *username){
    int k = find_index(username);
    if (k<0) return -1;
    g_customers[k] = g_customers[g_customerCount-1]; // 尾部覆盖
    g_customerCount--;
    return 0;
}

int editCustomer(const char *username, const char *newPhone, float newBalance){
    int k = find_index(username);
    if (k<0) return -1;
    if (newPhone && newPhone[0]){
        if (!phone_ok(newPhone)) return -2;
        strncpy(g_customers[k].phone, newPhone, sizeof(g_customers[k].phone)-1);
    }
    if (newBalance < 0) return -3;
    g_customers[k].balance = newBalance;
    return 0;
}

int updateCustomerBalanceAndOrders(const char *username, float deltaBalance, int deltaOrders){
    int k = find_index(username);
    if (k<0) return -1;
    float nb = g_customers[k].balance + deltaBalance;
    int   no = g_customers[k].orderCount + deltaOrders;
    if (nb < 0) return -2;
    if (no < 0) no = 0;
    g_customers[k].balance    = nb;
    g_customers[k].orderCount = no;
    return 0;
}
