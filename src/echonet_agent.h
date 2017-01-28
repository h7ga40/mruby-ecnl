/*
 *  TOPPERS ECHONET Lite Communication Middleware
 * 
 *  Copyright (C) 2015-2016 Cores Co., Ltd. Japan
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
 *		ECHONET Lite 動的生成ノード
 */

#ifndef ECHONET_NODE_H
#define ECHONET_NODE_H

#include "echonet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  キューのデータ構造の定義
 */
typedef struct ecn_agent_queue {
#ifdef _DEBUG
	struct ecn_agent_queue *p_parent;	/* 親キューへのポインタ */
#endif
	struct ecn_agent_queue *p_next;		/* 次エントリへのポインタ */
	struct ecn_agent_queue *p_prev;		/* 前エントリへのポインタ */
} ecn_agent_queue_t;

#define PMAP_FLAG_SET	0x01
#define PMAP_FLAG_GET	0x02
#define PMAP_FLAG_ANNO	0x04

typedef struct ecn_obj
{
	ecnl_svc_task_t *svc;
	ID eobjId;
	EOBJINIB inib;
	uint8_t pmapFlag;
	uint8_t pmapSet[16];		/* SETプロパティマップ */
	uint8_t pmapGet[16];		/* GETプロパティマップ */
	uint8_t pmapAnno[16];		/* 状態変化時通知プロパティマップ */
	uint_t eprpcnt;				/* ECHONET Lite プロパティ数 */
} ecn_obj_t;

typedef struct ecn_node
{
	ecn_obj_t base;
	EOBJCB eobj;
	ECN_ENOD_ID enodId;
	ecn_agent_queue_t devices;
} ecn_node_t;

typedef struct ecn_device {
	ecn_obj_t base;
	ecn_node_t *node;
} ecn_device_t;

typedef enum ecn_node_state {
	ecn_node_state_idle,
	ecn_node_state_start,
	ecn_node_state_set_prpmap_wait,
	ecn_node_state_get_prpmap_wait,
	ecn_node_state_anno_prpmap_wait,
	ecn_node_state_fault,
} ecn_node_state_t;

typedef struct ecn_node_agent {
	ecn_node_t node;
	ecn_node_state_t state;
	TMO timer;
	ecn_obj_t *current;
	int retry;
} ecn_node_agent_t;

typedef struct ecn_agent_buffer {
	union {
		struct ecn_agent_buffer *free;
		ecn_agent_queue_t queue;
	}hdr;
	union {
		ecn_obj_t obj;
		ecn_node_agent_t node;
		ecn_device_t device;
	}cnt;
} ecn_agent_buffer_t;

#define TNUM_AEOBJID 50

typedef struct ecn_agent {
	EPRPINIB eprpinib;
	const EPRPINIB *peprpinib;
	ecn_agent_queue_t nodes;
	ecn_node_agent_t *current_node;
	bool_t msg_proced;
	int blkpos;
	ecn_agent_buffer_t *free;
	ecn_agent_buffer_t blockes[TNUM_AEOBJID];
	int requestid;
} ecn_agent_t;

typedef struct ecn_inm_get_device_list_req
{
	unsigned int requestid;
} ecn_inm_get_device_list_req_t;

typedef struct ecn_inm_get_device_item
{
	ID			eobjid;			/* ECHONET Lite オブジェクトID */
	ID			enodid;			/* ECHONET Lite ノードプロファイルオブジェクトID */
	uint8_t		state;			/* 情報収集状態 */
	uint8_t		eojx1;			/* ECHONET Lite オブジェクトのクラスグループコード */
	uint8_t		eojx2;			/* ECHONET Lite オブジェクトのクラスコード */
	uint8_t		eojx3;			/* ECHONET Lite オブジェクトのインスタンスコード */
	ECN_ENOD_ID	addrid;			/* 通信レイヤーアドレス取得用ID */
} ecn_inm_get_device_item_t;

typedef struct ecn_inm_get_device_list_res
{
	unsigned int requestid;
	ecn_inm_get_device_item_t devices[1];
} ecn_inm_get_device_list_res_t;

typedef struct ecn_inm_get_device_info_req
{
	unsigned int requestid;
	ID eobjid;
} ecn_inm_get_device_info_req_t;

typedef struct ecn_inm_get_device_info_res
{
	unsigned int requestid;
	ID eobjid;
	uint8_t pmapSet[16];
	uint8_t pmapGet[16];
	uint8_t pmapAnno[16];
	uint_t eprpcnt;
} ecn_inm_get_device_info_res_t;

/* 初期化 */
void ecn_agent_init(ecnl_svc_task_t *svc);
/* ノードを検索 */
ecn_node_t *ecn_agent_find_node(ecnl_svc_task_t *svc, ECN_ENOD_ID enodid);
/* 機器を検索 */
ecn_obj_t *ecn_agent_find_eobj(const EOBJCB *pk_nod, T_ECN_EOJ eoj);
/* オブジェクトIDを取得 */
ID ecn_agent_get_eobj(const EOBJINIB *pk_obj);
/* 次の機器を取得 */
const EOBJINIB *ecn_agent_next_eobj(const EOBJCB *pk_nod, const EOBJINIB *pk_obj);
/* オブジェクトIDからEOJとノードIDを取得 */
bool_t ecn_agent_get_eoj_enodid(ecnl_svc_task_t *svc, ID eobjid, T_ECN_EOJ *eoj, ECN_ENOD_ID *enodid);
/* タイムアウト値を返す */
TMO ecn_agent_get_timer(ecnl_svc_task_t *svc);
/* タイマーの時間を進める */
void ecn_agent_progress(ecnl_svc_task_t *svc, TMO interval);
/* タイムアウト処理（のための呼び出し） */
void ecn_agent_timeout(ecnl_svc_task_t *svc);
/* 内部メッセージ処理 */
bool_t ecn_agent_proc_int_msg(ecnl_svc_task_t *svc, ECN_FBS_ID fbs, uint8_t cmd);
/* ECHONET電文受信処理 */
void ecn_agent_proc_ecn_msg(ecnl_svc_task_t *svc, const EOBJCB **snod, const EOBJINIB **sobj, T_EDATA *esv);
/* ECHONET電文受信処理終了 */
void ecn_agent_proc_ecn_msg_end(ecnl_svc_task_t *svc);
/* プロパティを取得 */
const EPRPINIB *ecn_agent_get_property(const EOBJINIB *fp_obj, uint8_t fa_epc, const EPRPINIB *item);

ER ecn_agent_get_device_list(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, int requestid, ECN_FBS_ID *pk_req);
ER ecn_agent_get_device_info(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, int requestid, ID eobjid, ECN_FBS_ID *pk_req);

void ecn_agent_set_epc_to_prop_map(uint8_t epc, uint8_t *propMap);
bool_t ecn_agent_contains_epc_in_prop_map(uint8_t epc, uint8_t *propMap);

#ifdef __cplusplus
}
#endif

#endif /* ECHONET_NODE_H */
