// models.h
#pragma once

typedef struct {
    char  username[30];
    char  phone[20];
    float balance;     // 元为单位（显示/交互用）
    int   orderCount;  // 历史订单数
} Customer;
