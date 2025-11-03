// user_store_csv.h
#pragma once
#include "models.h"

#ifdef __cplusplus
extern "C" {
#endif

// 生命周期（与项目命名一致）
int  loadData(void);   // 读取 customers.csv 到内存（若文件不存在则创建空文件）
void saveData(void);   // 将内存写回 customers.csv（原子写：.tmp -> 覆盖）

// 读
int  listCustomers(Customer *out, int maxn);                 // 返回实际写入条数
int  getCustomerByName(const char *username, Customer *out); // 找到返回1，否则0

// 统一添加函数：店长端 & 顾客注册共用
typedef enum { ACTOR_CUSTOMER=0, ACTOR_MANAGER=1 } Actor;
int  addCustomer(const char *username, const char *phone,
                 float initialBalance, Actor actor);

// 仅店长端调用的修改/删除
int  removeCustomer(const char *username);
int  editCustomer(const char *username, const char *newPhone, float newBalance);

// 下单后余额与订单数联动（余额变化 + 订单数增量）；余额不可为负
int  updateCustomerBalanceAndOrders(const char *username,
                                    float deltaBalance, int deltaOrders);

#ifdef __cplusplus
}
#endif
