/*
 * Copyright 2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#ifndef OSSL_QUIC_ACKM_H
#define OSSL_QUIC_ACKM_H

#include "internal/quic_statm.h"
#include "internal/quic_cc.h"
#include "internal/quic_types.h"
#include "internal/quic_wire.h"
#include "internal/time.h"
#include "internal/list.h"

typedef struct ossl_ackm_st OSSL_ACKM;

OSSL_ACKM *ossl_ackm_new(OSSL_TIME (*now)(void *arg), void *now_arg,
                         OSSL_STATM *statm, const OSSL_CC_METHOD *cc_method,
                         OSSL_CC_DATA *cc_data);
void ossl_ackm_free(OSSL_ACKM *ackm);

void ossl_ackm_set_loss_detection_deadline_callback(
    OSSL_ACKM *ackm, void (*fn)(OSSL_TIME deadline, void *arg), void *arg);

void ossl_ackm_set_ack_deadline_callback(OSSL_ACKM *ackm,
                                         void (*fn)(OSSL_TIME deadline,
                                                    int pkt_space, void *arg),
                                         void *arg);

typedef struct ossl_ackm_tx_pkt_st OSSL_ACKM_TX_PKT;
struct ossl_ackm_tx_pkt_st
{
  /* The packet number of the transmitted packet. */
  QUIC_PN pkt_num;

  /* The number of bytes in the packet which was sent. */
  size_t num_bytes;

  /* The time at which the packet was sent. */
  OSSL_TIME time;

  /*
   * If the packet being described by this structure contains an ACK frame,
   * this must be set to the largest PN ACK'd by that frame.
   *
   * Otherwise, it should be set to QUIC_PN_INVALID.
   *
   * This is necessary to bound the number of PNs we have to keep track of on
   * the RX side (RFC 9000 s. 13.2.4). It allows older PN tracking information
   * on the RX side to be discarded.
   */
  QUIC_PN largest_acked;

  /*
   * One of the QUIC_PN_SPACE_* values. This qualifies the pkt_num field
   * into a packet number space.
   */
  unsigned int pkt_space : 2;

  /* 1 if the packet is in flight. */
  unsigned int is_inflight : 1;

  /* 1 if the packet has one or more ACK-eliciting frames. */
  unsigned int is_ack_eliciting : 1;

  /* 1 if the packet is a PTO probe. */
  unsigned int is_pto_probe : 1;

  /* 1 if the packet is an MTU probe. */
  unsigned int is_mtu_probe : 1;

  /* Callback called if frames in this packet are lost. arg is cb_arg. */
  void (*on_lost)(void *arg);
  /* Callback called if frames in this packet are acked. arg is cb_arg. */
  void (*on_acked)(void *arg);
  /*
   * Callback called if frames in this packet are neither acked nor lost. arg
   * is cb_arg.
   */
  void (*on_discarded)(void *arg);
  void *cb_arg;

  /*
   * (Internal use fields; must be zero-initialized.)
   *
   * Keep a TX history list, anext is used to manifest
   * a singly-linked list of newly-acknowledged packets, and lnext is used to
   * manifest a singly-linked list of newly lost packets.
   */
  OSSL_LIST_MEMBER(tx_history, OSSL_ACKM_TX_PKT);

  struct ossl_ackm_tx_pkt_st *anext;
  struct ossl_ackm_tx_pkt_st *lnext;
};

int ossl_ackm_on_tx_packet(OSSL_ACKM *ackm, OSSL_ACKM_TX_PKT *pkt);
int ossl_ackm_on_rx_datagram(OSSL_ACKM *ackm, size_t num_bytes);

#define OSSL_ACKM_ECN_NONE 0
#define OSSL_ACKM_ECN_ECT1 1
#define OSSL_ACKM_ECN_ECT0 2
#define OSSL_ACKM_ECN_ECNCE 3

typedef struct ossl_ackm_rx_pkt_st
{
  /* The packet number of the received packet. */
  QUIC_PN pkt_num;

  /* The time at which the packet was received. */
  OSSL_TIME time;

  /*
   * One of the QUIC_PN_SPACE_* values. This qualifies the pkt_num field
   * into a packet number space.
   */
  unsigned int pkt_space : 2;

  /* 1 if the packet has one or more ACK-eliciting frames. */
  unsigned int is_ack_eliciting : 1;

  /*
   * One of the OSSL_ACKM_ECN_* values. This is the ECN labelling applied to
   * the received packet. If unknown, use OSSL_ACKM_ECN_NONE.
   */
  unsigned int ecn : 2;
} OSSL_ACKM_RX_PKT;

int ossl_ackm_on_rx_packet(OSSL_ACKM *ackm, const OSSL_ACKM_RX_PKT *pkt);

int ossl_ackm_on_rx_ack_frame(OSSL_ACKM *ackm, const OSSL_QUIC_FRAME_ACK *ack,
                              int pkt_space, OSSL_TIME rx_time);

/*
 * Discards a PN space. This must be called for a PN space before freeing the
 * ACKM if you want in-flight packets to have their discarded callbacks called.
 * This should never be called in ordinary QUIC usage for the Application Data
 * PN space, but it may be called for the Application Data PN space prior to
 * freeing the ACKM to simplify teardown implementations.
 */
int ossl_ackm_on_pkt_space_discarded(OSSL_ACKM *ackm, int pkt_space);

int ossl_ackm_on_handshake_confirmed(OSSL_ACKM *ackm);
int ossl_ackm_on_timeout(OSSL_ACKM *ackm);

OSSL_TIME ossl_ackm_get_loss_detection_deadline(OSSL_ACKM *ackm);

/*
 * Generates an ACK frame, regardless of whether the ACK manager thinks
 * one should currently be sent.
 *
 * This clears the flag returned by ossl_ackm_is_ack_desired and the deadline
 * returned by ossl_ackm_get_ack_deadline.
 */
const OSSL_QUIC_FRAME_ACK *ossl_ackm_get_ack_frame(OSSL_ACKM *ackm,
                                                   int pkt_space);

/*
 * Returns the deadline after which an ACK frame should be generated by calling
 * ossl_ackm_get_ack_frame, or OSSL_TIME_INFINITY if no deadline is currently
 * applicable. If the deadline has already passed, this function may return that
 * deadline, or may return OSSL_TIME_ZERO.
 */
OSSL_TIME ossl_ackm_get_ack_deadline(OSSL_ACKM *ackm, int pkt_space);

/*
 * Returns 1 if the ACK manager thinks an ACK frame ought to be generated and
 * sent at this time. ossl_ackm_get_ack_frame will always provide an ACK frame
 * whether or not this returns 1, so it is suggested that you call this function
 * first to determine whether you need to generate an ACK frame.
 *
 * The return value of this function can change based on calls to
 * ossl_ackm_on_rx_packet and based on the passage of time (see
 * ossl_ackm_get_ack_deadline).
 */
int ossl_ackm_is_ack_desired(OSSL_ACKM *ackm, int pkt_space);

/*
 * Returns 1 if the given RX PN is 'processable'. A processable PN is one that
 * is not either
 *
 *   - duplicate, meaning that we have already been passed such a PN in a call
 *     to ossl_ackm_on_rx_packet; or
 *
 *   - written off, meaning that the PN is so old we have stopped tracking state
 *     for it (meaning that we cannot tell whether it is a duplicate and cannot
 *     process it safely).
 *
 * This should be called for a packet before attempting to process its contents.
 * Failure to do so may result in processing a duplicated packet in violation of
 * the RFC.
 *
 * The return value of this function transitions from 1 to 0 for a given PN once
 * that PN is passed to ossl_ackm_on_rx_packet, thus thus function must be used
 * before calling ossl_ackm_on_rx_packet.
 */
int ossl_ackm_is_rx_pn_processable(OSSL_ACKM *ackm, QUIC_PN pn, int pkt_space);

typedef struct ossl_ackm_probe_info_st
{
  uint32_t handshake;
  uint32_t padded_initial;
  uint32_t pto[QUIC_PN_SPACE_NUM];
} OSSL_ACKM_PROBE_INFO;

int ossl_ackm_get_probe_request(OSSL_ACKM *ackm, int clear,
                                OSSL_ACKM_PROBE_INFO *info);

int ossl_ackm_get_largest_unacked(OSSL_ACKM *ackm, int pkt_space, QUIC_PN *pn);

#endif
