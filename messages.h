#pragma once

enum {
    CLK_REQ_FACE_TIME = 1,
    CLK_REQ_TIME,
    CLK_ACCEPT_STATUS
};

enum {
    SRVR_ACCEPT_FACE_TIME = 1,
    SRVR_ACCEPT_TIME,
    SRVR_ACCEPT_DONE,
};

typedef struct {
    uint8_t cmd;
    uint8_t timeouts;
    uint16_t battery : 12;
    uint16_t interval : 4;
    uint16_t match;
    int64_t offset;
} __attribute__((packed)) msg_sts_t;

typedef struct {
    uint8_t  cmd;
    uint64_t data;
} __attribute__((packed)) msg_u64_t;

typedef struct {
    uint8_t  cmd;
    uint16_t data;
} __attribute__((packed)) msg_u16_t;

typedef struct {
    uint8_t  cmd;
} __attribute__((packed)) msg_u8_t;

typedef union {
    msg_sts_t sts;
    msg_u64_t u64;
    msg_u16_t u16;
    msg_u8_t u8;
} __attribute__((packed)) msg_t;

