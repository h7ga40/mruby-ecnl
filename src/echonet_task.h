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

#ifndef TOPPERS_ECHONET_TASK_H
#define TOPPERS_ECHONET_TASK_H

/*#include ".h"*/

#include "echonet.h"
#include "echonet_fbs.h"
#include "echonet_agent.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ECHONET Lite タスク関連の定数のデフォルト値の定義
 */ 
#ifndef ECHONET_TASK_PRIORITY
#define ECHONET_TASK_PRIORITY	6		/* 初期優先度 */
#endif /* ECHONET_TASK_PRIORITY */

#ifndef ECHONET_TASK_STACK_SIZE
#define ECHONET_TASK_STACK_SIZE	1024	/* スタック領域のサイズ */
#endif /* ECHONET_TASK_STACK_SIZE */

#ifndef NUM_ECHONET_MAILBOX
#define NUM_ECHONET_MAILBOX	10
#endif /* NUM_ECHONET_MAILBOX */

#ifndef NUM_ECHONET_API_MAILBOX
#define NUM_ECHONET_API_MAILBOX	10
#endif /* NUM_ECHONET_API_MAILBOX */

#define TNUM_ENODADR 100

struct ecnl_svc_task {
	mrb_state *mrb;
	mrb_value self;
	intptr_t exinf;
	/* アプリケーションが要求した電文のシーケンス番号 */
	uint16_t api_tid;
	/* 受信メッセージを開放するか否か */
	bool_t release_esv;
	/* 受信メッセージを転送するか否か */
	bool_t forward_esv;
	/* コントローラー向け */
	ecn_agent_t agent;
	/* ECHONET Lite オブジェクトIDの最大値 */
	ID tmax_eobjid;
	/* ECHONET Lite オブジェクト初期化ブロックのエリア */
	const EOBJINIB **eobjinib_table;
	/* ECHONET Liteノード管理ブロックの数 */
	int tnum_enodid;
	/* ECHONET Lite オブジェクト管理ブロックのエリア */
	EOBJCB eobjcb_table[1];
	/* ECHONET Liteノードと通信レイヤーアドレスの対応情報の数 */
	int tnum_enodadr;
	/* ECHONET Lite ノードと通信レイヤーアドレスの対応情報ブロックのエリア */
	ENODADRB enodadrb_table[TNUM_ENODADR];
	/* メールボックス */
	T_ECN_FBS_QUEUE api_mbxid;
	T_ECN_FBS_QUEUE svc_mbxid;
	T_ECN_FBS_QUEUE lcl_mbxid;
	/* モジュール初期化フラグ */
	uint8_t eobjlist_need_init;
	/* シーケンス番号 */
	uint16_t current_tid;
};

TMO echonet_svctask_get_timer(ecnl_svc_task_t *svc);
void echonet_svctask_progress(ecnl_svc_task_t *svc, TMO a_timer);
void echonet_svctask_recv_msg(ecnl_svc_task_t *svc, T_ECN_FST_BLK *p_msg);
void echonet_svctask_timeout(ecnl_svc_task_t *svc);

const EOBJINIB *echonet_svctask_get_eobjinib(ecnl_svc_task_t *svc, ID eobjid);
ID echonet_svctask_get_eobjid(ecnl_svc_task_t *svc, const EOBJINIB *eobjinib);

/* ECHONET Lite インスタンスリストの1アナウンス当たり最大ノード数 */
#define ECN_IST_LST_EOJ_MAX_CT (84)

/* ECHONET Liteオブジェクトコード x1,x2,x3(T_ECN_EOJ)を24bit整数に変換 */
#define _ECN_EOJ2VALUE(s) (((s).eojx1 << 16) | ((s).eojx2 << 8) | (s).eojx3)

enum ecn_epc_code
{
	ECN_EPC_INL_BCS		=	0xD5	/* インスタンスリスト通知プロパティ */
};

typedef enum
{
	ECN_MSG_UNDEF		=	0,
	ECN_MSG_INTERNAL	=	1,
	ECN_MSG_ECHONET		=	2,
	ECN_MSG_USER_BREAK	=	3
} ECN_MSG_TYPE;

typedef enum
{
	ECN_INM_NOTIFY_INSTANCELIST = 1,
	ECN_INM_GET_DEVICE_LIST_REQ,
	ECN_INM_GET_DEVICE_LIST_RES,
	ECN_INM_GET_DEVICE_INFO_REQ,
	ECN_INM_GET_DEVICE_INFO_RES,
	ECN_LCL_MSG_GET_LCLADDR_REQ,
	ECN_LCL_MSG_GET_LCLADDR_RES,
	ECN_LCL_MSG_GET_LCLADDR_ERROR
} ECN_INT_MSG_COMMAND;

typedef struct
{
	uint8_t	command;
	uint8_t	data[1];
} T_ECN_INTERNAL_MSG;

/* タスク初期化 */
void _ecn_tsk_int_startup(ecnl_svc_task_t *svc);
/* インスタンスリスト通知の送信 */
ER _ecn_tsk_ntf_inl(ecnl_svc_task_t *svc);

/*
 * 要求電文作成
 */
ER _ecn_tsk_mk_esv(ecnl_svc_task_t *svc, ECN_FBS_ID *fp_fbs_id, ID fa_seoj, ID fa_deoj,
	uint8_t fa_epc, uint8_t fa_pdc, const void *p_edt, ECN_SRV_CODE fa_esv);

ER _ecn_tsk_snd_mbx(ecnl_svc_task_t *svc, ECN_FBS_ID fa_rsp_fbs, bool_t from_app);

/*
 * 応答電文待ちの割り込み電文作成
 */
ER _ecn_mk_brk_wai(ecnl_svc_task_t *svc, ECN_FBS_ID *pk_fbs_id, const void *p_dat, size_t fa_size);

const EOBJCB *_ecn_eno_fnd(ecnl_svc_task_t *svc, ECN_ENOD_ID enodid);

const EOBJINIB *_ecn_eoj_fnd(const EOBJCB *fp_nod, const T_ECN_EOJ *fp_eoj);

ER _ecn_tsk_cre_req_fbs(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, uint8_t cmd, ECN_FBS_ID *pk_req);

ER _ecn_tsk_cre_res_fbs(ecnl_svc_task_t *svc, ECN_FBS_ID req, uint8_t cmd, ECN_FBS_ID *pk_res);

#ifdef __cplusplus
}
#endif

#endif /* TOPPERS_ECHONET_TASK_H */
