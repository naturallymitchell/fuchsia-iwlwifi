// Copyright (c) 2019 The Fuchsia Authors.
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FUCHSIA_PORTING_H_
#define SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FUCHSIA_PORTING_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/if_ether.h>
#include <zircon/compiler.h>
#include <zircon/types.h>

#include "src/connectivity/wlan/drivers/third_party/intel/iwlwifi/ieee80211.h"

typedef uint32_t __be32;
typedef uint32_t __be16;
typedef uint64_t __le64;
typedef uint32_t __le32;
typedef uint16_t __le16;
typedef uint8_t __s8;
typedef uint8_t __u8;

typedef uint32_t netdev_features_t;

typedef uint64_t dma_addr_t;

typedef char* acpi_string;

#define BIT(x) (1 << (x))

#define DIV_ROUND_UP(num, div) (((num) + (div)-1) / (div))

#define le32_to_cpu(x) (x)
#define le32_to_cpup(x) (*x)
#define cpu_to_le32(x) (x)

#define BITS_PER_LONG (sizeof(long) * 8)

#define BITS_TO_LONGS(nr) (nr / BITS_PER_LONG)

#define ETHTOOL_FWVERS_LEN 32

#define IS_ENABLED(x) (false)

#define lockdep_assert_held(x) do {} while (0)
#define pr_err(fmt, args...) zxlogf(ERROR, fmt, args)
#define __aligned(x) __attribute__((aligned(x)))
#define __bitwise
#define __exit
#define __force
#define __init
#define __must_check __attribute__((warn_unused_result))
#define __packed __PACKED
#define __rcu  // NEEDS_PORTING
#define ____cacheline_aligned_in_smp  // NEEDS_PORTING

// NEEDS_PORTING: Need to check if 'x' is static array.
#define ARRAY_SIZE(x) (countof(x))

// NEEDS_PORTING
#define WARN(x, y, z) \
    do {              \
    } while (0)
#define WARN_ON(x) (false)
#define WARN_ON_ONCE(x) (false)
#define BUILD_BUG_ON(x) (false)

#define offsetofend(type, member) (offsetof(type, member) + sizeof(((type*)NULL)->member))

// NEEDS_PORTING: need to be generic
#define roundup_pow_of_two(x)     \
    (x >= 0x100 ? 0xFFFFFFFF :    \
     x >= 0x080 ? 0x100 :         \
     x >= 0x040 ? 0x080 :         \
     x >= 0x020 ? 0x040 :         \
     x >= 0x010 ? 0x020 :         \
     x >= 0x008 ? 0x010 :         \
     x >= 0x004 ? 0x008 :         \
     x >= 0x002 ? 0x004 :         \
     x >= 0x001 ? 0x002 : 1)


// NEEDS_PORTING: need protection while accessing the variable.
#define rcu_dereference(p) (p)
#define rcu_dereference_protected(p, c) (p)

// NEEDS_PORTING: how to guarantee this?
#define READ_ONCE(x) (x)

// NEEDS_PORTING: implement this
#define rcu_read_lock() do {} while (0);
#define rcu_read_unlock() do {} while (0);

// NEEDS_PORTING: Below structures are used in code but not ported yet.
struct delayed_work {};
struct dentry {};
struct device {};
struct ewma_rate {};
struct inet6_dev {};
struct mac_address {};
struct napi_struct {};
struct rcu_head {};
struct sk_buff_head {};
struct timer_list {};
struct wait_queue {};
struct wait_queue_head {};
struct wiphy {};
struct work_struct {};

struct firmware {
    zx_handle_t vmo;
    uint8_t* data;
    size_t size;
};

struct page {
    void* virtual;
};

struct wireless_dev {
    enum nl80211_iftype iftype;
};

////
// Typedefs
////
typedef struct wait_queue wait_queue_t;
typedef struct wait_queue_head wait_queue_head_t;

////
// Inlines functions
////

static inline int test_bit(int nbits, const volatile unsigned long* addr) {
    return 1UL & (addr[nbits / BITS_PER_LONG] >> (nbits % BITS_PER_LONG));
}

static inline void set_bit(int nbits, unsigned long* addr) {
    addr[nbits / BITS_PER_LONG] |= 1UL << (nbits % BITS_PER_LONG);
}
#define __set_bit set_bit

static inline void clear_bit(int nbits, volatile unsigned long* addr) {
    addr[nbits / BITS_PER_LONG] &= ~(1UL << (nbits % BITS_PER_LONG));
}

static inline void might_sleep() {}

static inline void* vmalloc(unsigned long size) {
    return malloc(size);
}

static inline void* kmemdup(const void* src, size_t len) {
    void* dst = malloc(len);
    memcpy(dst, src, len);
    return dst;
}

static inline void vfree(const void* addr) {
    free((void*)addr);
}

static inline void kfree(void* addr) {
    free(addr);
}

static inline bool IS_ERR_OR_NULL(const void *ptr) {
    return !ptr || (((unsigned long)ptr) >= (unsigned long)-4095);
}

static inline void *page_address(const struct page *page)
{
    return page->virtual;
}

#endif  // SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FUCHSIA_PORTING_H_
