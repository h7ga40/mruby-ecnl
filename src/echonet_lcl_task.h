/*
 *  TOPPERS ECHONET Lite Communication Middleware
 * 
 *  Copyright (C) 2014-2016 Cores Co., Ltd. Japan
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 */

/*
 *		ECHONET Lite タスク
 */

#ifndef TOPPERS_ECHONET_LCL_TASK_H
#define TOPPERS_ECHONET_LCL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mruby.h>
#include "echonet.h"
#include "echonet_fbs.h"

/*
 *  ECHONET Lite 一斉同報アドレス
 */
#ifdef SIL_ENDIAN_LITTLE
#define ECHONET_MULTICAST_ADDR {0,23,0,224}
#else
#define ECHONET_MULTICAST_ADDR {224,0,23,0}
#endif

/*
 *  ECHONET Lite タスク関連の定数のデフォルト値の定義
 */
#ifndef ECHONET_LCL_TASK_PRIORITY
#define ECHONET_LCL_TASK_PRIORITY	3		/* 初期優先度 */
#endif /* ECHONET_LCL_TASK_PRIORITY */

#ifndef ECHONET_LCL_TASK_STACK_SIZE
#define ECHONET_LCL_TASK_STACK_SIZE	1024	/* スタック領域のサイズ */
#endif /* ECHONET_LCL_TASK_STACK_SIZE */

#ifndef NUM_ECHONET_LCL_MAILBOX
#define NUM_ECHONET_LCL_MAILBOX	10
#endif /* NUM_ECHONET_LCL_MAILBOX */

 /* ECHONET Lite 通信レイヤータスク タイマー値取得 */
TMO echonet_lcltask_get_timer(ecnl_svc_task_t *svc);

/* ECHONET Lite 通信レイヤータスク 時間経過処理 */
void echonet_lcltask_progress(ecnl_svc_task_t *svc, TMO a_timer);

/* ECHONET Lite 通信レイヤータスク メッセージ受信処理 */
void echonet_lcltask_recv_msg(ecnl_svc_task_t *svc, T_ECN_FST_BLK *p_msg);

/* ECHONET Lite 通信レイヤータスク タイムアウト処理 */
void echonet_lcltask_timeout(ecnl_svc_task_t *svc);

/* ECHONET Lite 通信レイヤーメッセージ入力 */
ER echonet_lcl_input_msg(ecnl_svc_task_t *svc, mrb_value ep, mrb_value data);

/*
 *  リモートECHONETノードの適合確認
 */
bool_t ecn_is_match(ecnl_svc_task_t *svc, const EOBJCB *eobjcb, T_EDATA *edata, mrb_value ep);

typedef struct ecn_lcl_msg_get_lcladdr_req
{
	unsigned int requestid;
	ECN_ENOD_ID enodid;
} ecn_lcl_msg_get_lcladdr_req_t;

typedef struct ecn_lcl_msg_get_lcladdr_res
{
	unsigned int requestid;
	ECN_ENOD_ID enodid;
	mrb_value enodadrb;
} ecn_lcl_msg_get_lcladdr_res_t;

typedef struct ecn_lcl_msg_get_lcladdr_error
{
	unsigned int requestid;
	ER error;
} ecn_lcl_msg_get_lcladdr_error_t;

ER ecn_lcl_get_lcladdr(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, int requestid, ECN_ENOD_ID enodid, ECN_FBS_ID *pk_req);

#ifdef __cplusplus
}
#endif

#endif /* TOPPERS_ECHONET_LCL_TASK_H */
