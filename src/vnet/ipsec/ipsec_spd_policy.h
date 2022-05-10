/*
 * Copyright (c) 2015 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __IPSEC_SPD_POLICY_H__
#define __IPSEC_SPD_POLICY_H__

#include <vppinfra/bihash_40_8.h>
#include <vppinfra/bihash_16_8.h>
#include <vnet/ipsec/ipsec_spd.h>
/**
 * calculated as max number of flows (2^10) divided by KVP_PER_PAGE (4)
 */
#define IPSEC_FP_HASH_LOOKUP_HASH_BUCKETS (1 << 8)

#define IPSEC_POLICY_PROTOCOL_ANY IP_PROTOCOL_RESERVED

#define foreach_ipsec_policy_action \
  _ (0, BYPASS, "bypass")           \
  _ (1, DISCARD, "discard")         \
  _ (2, RESOLVE, "resolve")         \
  _ (3, PROTECT, "protect")

typedef enum
{
#define _(v, f, s) IPSEC_POLICY_ACTION_##f = v,
  foreach_ipsec_policy_action
#undef _
} ipsec_policy_action_t;

#define IPSEC_POLICY_N_ACTION (IPSEC_POLICY_ACTION_PROTECT + 1)

typedef struct
{
  ip46_address_t start, stop;
} ip46_address_range_t;

typedef struct
{
  u16 start, stop;
} port_range_t;

/**
 * @brief
 * Policy packet & bytes counters
 */
extern vlib_combined_counter_main_t ipsec_spd_policy_counters;

/**
 * @brief A Secruity Policy. An entry in an SPD
 */
typedef struct ipsec_policy_t_
{
  u32 id;
  i32 priority;

  // the type of policy
  ipsec_spd_policy_type_t type;

  // Selector
  u8 is_ipv6;
  ip46_address_range_t laddr;
  ip46_address_range_t raddr;
  u8 protocol;
  port_range_t lport;
  port_range_t rport;

  // Policy
  ipsec_policy_action_t policy;
  u32 sa_id;
  u32 sa_index;
} ipsec_policy_t;

/**
 * @brief Add/Delete a SPD
 */
extern int ipsec_add_del_policy (vlib_main_t * vm,
				 ipsec_policy_t * policy,
				 int is_add, u32 * stat_index);

extern u8 *format_ipsec_policy (u8 * s, va_list * args);
extern u8 *format_ipsec_policy_action (u8 * s, va_list * args);
extern uword unformat_ipsec_policy_action (unformat_input_t * input,
					   va_list * args);


extern int ipsec_policy_mk_type (bool is_outbound,
				 bool is_ipv6,
				 ipsec_policy_action_t action,
				 ipsec_spd_policy_type_t * type);

/* A 5-tuple used to calculate the bihash entry  */
typedef union
{
  struct
  {
    union
    {
      struct
      {
	u32 l3_zero_pad[6];
	ip4_address_t laddr;
	ip4_address_t raddr;
      };
      ip6_address_t ip6_laddr;
      ip6_address_t ip6_raddr;
    };

    u16 lport;
    u16 rport;
    u16 protocol;
    u16 is_ipv6;
  };
  /* for ipv6 */
  clib_bihash_kv_40_8_t kv_40_8;
  /* for ipv4 */
  struct
  {
    u64 padding_for_kv_16_8[3];
    clib_bihash_kv_16_8_t kv_16_8;
  };
} ipsec_fp_5tuple_t;

/*
 * An element describing a particular policy  mask,
 * and refcount of policies with same mask.
 */
typedef struct
{
  /** Required for pool_get_aligned */
  CLIB_CACHE_LINE_ALIGN_MARK (cacheline0);
  ipsec_fp_5tuple_t mask;
  u32 refcount; /* counts how many policies use this mask */
} ipsec_fp_mask_type_entry_t;

/*
 * Bihash lookup value,
 * contains an unordered vector of policies indices in policy pool.
 */
typedef union
{
  u64 as_u64;
  struct
  {
    u32 *fp_policies_ids;
  };
} ipsec_fp_lookup_value_t;

#endif /* __IPSEC_SPD_POLICY_H__ */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
