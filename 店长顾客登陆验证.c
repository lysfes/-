#include <stdio.h>
#include <string.h>

// ================== 结构体定义 ==================
struct Customer {
    char username[30];
    char phone[20];
    float balance;
    int orderCount;
};

struct Order {
    char username[30];
    char coffeeName[30];
    float totalPrice;
    char date[20];
};

struct Coffee {
    char name[30];
    float base_price;
    float bean_price;
    float milk_price;
    float extra_shot;
};

// ================== 全局变量 ==================
struct Customer customers[100];
int customerCount = 0;

struct Coffee coffees[50];
int coffeeCount = 0;

struct Order orders[500];
int orderCount = 0;

// ================== 登录函数 ==================
int managerLogin() {
    char u[30], p[30];
    printf("=== 店长登录 ===\n");
    printf("账号: ");
    scanf("%s", u);
    printf("密码: ");
    scanf("%s", p);

    if (strcmp(u, "admin") == 0 && strcmp(p, "123456") == 0) {
        printf("登录成功！\n");
        return 1;
    }
    printf("账号或密码错误！\n");
    return 0;
}

int customerLogin(int *index) {
    char u[30], ph[20];
    printf("=== 顾客登录 ===\n");
    printf("用户名: ");
    scanf("%s", u);
    printf("电话号: ");
    scanf("%s", ph);

    for (int i = 0; i < customerCount; i++) {
        if (strcmp(customers[i].username, u) == 0 &&
            strcmp(customers[i].phone, ph) == 0) {
            printf("登录成功！欢迎 %s\n", u);
            *index = i;
            return 1;
        }
    }

    printf("用户名或电话号错误！\n");
    return 0;
}

// ================== 登录后的菜单 ==================
void managerMenu() {
    int choice;
    printf("\n===== 店长菜单 =====\n");
    printf("1. 管理咖啡菜单（增删改咖啡）\n");
    printf("2. 管理顾客账户（增删改用户）\n");
    printf("3. 查看销售统计数据\n");
    printf("0. 返回主菜单\n");
    printf("请输入选项: ");
    scanf("%d", &choice);

    printf("（填入后续函数）\n");
}

void customerMenu(int index) {
    int choice;
    printf("\n===== 顾客菜单（%s） =====\n", customers[index].username);
    printf("1. 点单\n");
    printf("2. 查看订单历史\n");
    printf("3. 查询余额\n");
    printf("0. 返回主菜单\n");
    printf("请输入选项: ");
    scanf("%d", &choice);

    printf("（填入后续函数）\n");
}

// ================== 初始化一个测试的用户 ==================
void initData() {
    strcpy(customers[0].username, "alice");
    strcpy(customers[0].phone, "12345678901");
    customers[0].balance = 100;
    customers[0].orderCount = 0;
    customerCount = 1;
}

// ================== 主函数 ==================
int main() {
    int mainChoice;
    initData();

    while (1) {
        printf("\n===== 咖啡店系统 =====\n");
        printf("1. 店长登录\n");
        printf("2. 顾客登录\n");
        printf("0. 退出系统\n");
        printf("请输入选项: ");
        scanf("%d", &mainChoice);

        if (mainChoice == 1) {
            if (managerLogin()) {
                managerMenu();
            }
        }
        else if (mainChoice == 2) {
            int index;
            if (customerLogin(&index)) {
                customerMenu(index);
            }
        }
        else if (mainChoice == 0) {
            printf("系统已退出。\n");
            break;
        }
        else {
            printf("无效输入，请重新选择。\n");
        }
    }

    return 0;
}
