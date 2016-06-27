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
 *		ECHONET Lite LCL通信処理タスク
 */

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/string.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "echonet.h"

#include "echonet_task.h"
#include "echonet_lcl_task.h"
#include "echonet_dbg.h"
#include "mrb_ecnl.h"

#ifndef ECHONET_LCL_TASK_GET_TIMER
#define ECHONET_LCL_TASK_GET_TIMER	TMO_FEVR
#endif /* ECHONET_LCL_TASK_GET_TIMER */

#ifndef ECHONET_LCL_TASK_PROGRESS
#define ECHONET_LCL_TASK_PROGRESS(timer)
#endif /* ECHONET_LCL_TASK_PROGRESS */

#ifndef ECHONET_LCL_TASK_TIMEOUT
#define ECHONET_LCL_TASK_TIMEOUT
#endif /* ECHONET_LCL_TASK_TIMEOUT */

#ifndef ECHONET_LCL_SEND_TO_MBX
#define ECHONET_LCL_SEND_TO_MBX
#endif /* ECHONET_LCL_SEND_TO_MBX */

static ECN_ENOD_ID lcl_get_id(ecnl_svc_task_t *svc, T_EDATA *edata, const mrb_value ep);
static mrb_value lcl_get_addr(ecnl_svc_task_t *svc, ECN_ENOD_ID fa_enodid);
void _ecn_int_msg(ecnl_svc_task_t *svc, ECN_FBS_ID fbs_id, ECN_FBS_SSIZE_T a_snd_len);
void _ecn_esv_msg(ecnl_svc_task_t *svc, ECN_FBS_ID fbs_id);

/*
 * 受信したLCLデータをMAILBOXに送る
 */
ER _ecn_lcl2mbx(ecnl_svc_task_t *svc, const uint8_t *buffer, size_t fa_len, const mrb_value dst);
ER _ecn_lcl2mbx(ecnl_svc_task_t *svc, const uint8_t *buffer, size_t fa_len, const mrb_value dst)
{
	mrb_state *mrb = svc->mrb;
	ECN_FBS_ID	a_fbs_id = { 0 };
	ER			a_ret = E_OK;
	ECN_ENOD_ID	a_enod_id;
	union
	{
		const uint8_t			*buffer;
		const T_ECN_EDT_HDR		*t_esv;
	} a_rcv_pkt;

	a_rcv_pkt.buffer = buffer;
	if (	a_rcv_pkt.t_esv->ecn_hdr.ehd1 != ECN_EDH1_ECHONET_LITE	/* ECHONET Lite規格	*/
	||	a_rcv_pkt.t_esv->ecn_hdr.ehd2 != ECN_EDH2_FORMAT_1			/* 電文形式1		*/) {
		ECN_DBG_PUT_2("[LCL ECHO SRV] illegal type (0x%02X,0x%02X)", a_rcv_pkt.t_esv->ecn_hdr.ehd1, a_rcv_pkt.t_esv->ecn_hdr.ehd2);
		return E_PAR;
	}

#ifdef ECN_DBG_PUT_ENA
	_ecn_dbg_bindmp(buffer, fa_len);
#endif

	a_ret = _ecn_fbs_cre(mrb, fa_len, &a_fbs_id);
	if (a_ret != E_OK) {
		ECN_DBG_PUT_2("[LCL ECHO SRV] _ecn_fbs_cre() result = %d:%s", a_ret, itron_strerror(a_ret));
		return a_ret;
	}
	a_ret = _ecn_fbs_add_data_ex(mrb, a_fbs_id, buffer, fa_len);
	if (a_ret) {
		ECN_DBG_PUT_2("[LCL ECHO SRV] _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
		goto lb_except;
	}
	a_fbs_id.ptr->hdr.type = ECN_MSG_ECHONET;
	a_fbs_id.ptr->hdr.target.id = ENOD_LOCAL_ID;
	a_fbs_id.ptr->hdr.sender.id = ENOD_NOT_MATCH_ID;

	/* 通信レイヤーアドレスからリモートECHONETノードへ変換 */
	a_enod_id = lcl_get_id(svc, (T_EDATA *)a_fbs_id.ptr, dst);
	if (!lcl_is_valid_addrid(svc, a_enod_id)) {
		ECN_DBG_PUT_1("[LCL ECHO SRV] lcl src(%s) echonet-node not found.",
			ip2str(NULL, &dst->lcladdr));
	} else {
		/* 送信元ECHONETノードを記録 */
		a_fbs_id.ptr->hdr.sender.id = a_enod_id;
	}
	a_fbs_id.ptr->hdr.reply.id = a_fbs_id.ptr->hdr.sender.id;

	/* echonet_taskに送る */
	a_ret = ecn_fbs_enqueue(&svc->svc_mbxid, a_fbs_id.ptr);
	if (a_ret != E_OK) {
		ECN_DBG_PUT_2("[LCL ECHO SRV] snd_mbx(svc->svc_mbxid) result = %d:%s", a_ret, itron_strerror(a_ret));
		goto lb_except;
	}
	return a_ret;
lb_except:
	_ecn_fbs_del(mrb, a_fbs_id);
	return a_ret;
}

/*
* ECHONET Lite 通信レイヤータスク タイマー値取得
*/
TMO echonet_lcltask_get_timer(ecnl_svc_task_t *svc)
{
	TMO a_timer;

	a_timer = ECHONET_LCL_TASK_GET_TIMER;

	return a_timer;
}


/*
* ECHONET Lite 通信レイヤータスク 時間経過処理
*/
void echonet_lcltask_progress(ecnl_svc_task_t *svc, TMO a_timer)
{
	ECHONET_LCL_TASK_PROGRESS(a_timer);
}

/*
* ECHONET Lite 通信レイヤータスク メッセージ受信処理
*/
void echonet_lcltask_recv_msg(ecnl_svc_task_t *svc, T_ECN_FST_BLK *p_msg)
{
	ECN_FBS_ID a_fbs_id;
	ECN_FBS_SSIZE_T	a_snd_len;

	a_fbs_id.ptr = p_msg;

	/* 送信データ長を取得 */
	a_snd_len = _ecn_fbs_get_datalen(a_fbs_id);

	ECN_DBG_PUT_1("trcv_dtq() mbx recv (%d byte)", a_snd_len);

	switch (a_fbs_id.ptr->hdr.type) {
		/* 内部使用メッセージ */
	case ECN_MSG_INTERNAL:
		_ecn_int_msg(svc, a_fbs_id, a_snd_len);
		break;

		/* ECHONET用メッセージ */
	case ECN_MSG_ECHONET:
		_ecn_esv_msg(svc, a_fbs_id);
		break;

	default:
		ECN_DBG_PUT_1("echonet_task() a_fbs_id.ptr->k.hdr.k.t_edt.type:0x%02X undefined.", a_fbs_id.ptr->hdr.type);
	}

	/* 領域解放 */
	_ecn_fbs_del(svc->mrb, a_fbs_id);
}

/*
* ECHONET Lite 通信レイヤータスク タイムアウト処理
*/
void echonet_lcltask_timeout(ecnl_svc_task_t *svc)
{
	ECHONET_LCL_TASK_TIMEOUT;
}

/* 応答電文用fbs設定(sender/targetの設定) */
static ER _ecn_lcl_cre_req_fbs(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, uint8_t cmd, ECN_FBS_ID *pk_req)
{
	mrb_state *mrb = svc->mrb;
	ER ret;
	ECN_FBS_ID req;

	ret = _ecn_fbs_cre(mrb, 1, &req);
	if (ret != E_OK) {
		ECN_DBG_PUT_2("_ecn_lcl_cre_req_fbs() : _ecn_fbs_cre() result = %d:%s", ret, itron_strerror(ret));
		return ret;
	}

	ret = _ecn_fbs_add_data(mrb, req, &cmd, sizeof(cmd));
	if (ret != E_OK) {
		_ecn_fbs_del(mrb, req);
		ECN_DBG_PUT_2("_ecn_lcl_cre_req_fbs() : _ecn_fbs_add_data() result = %d:%s", ret, itron_strerror(ret));
		return ret;
	}

	req.ptr->hdr.type = ECN_MSG_INTERNAL;
	req.ptr->hdr.sender.mbxid = sender;
	req.ptr->hdr.target.mbxid = &svc->lcl_mbxid;
	req.ptr->hdr.reply.mbxid = sender;

	*pk_req = req;

	return E_OK;
}

/* 応答電文用fbs設定(sender/targetの設定) */
static ER _ecn_lcl_cre_res_fbs(ecnl_svc_task_t *svc, ECN_FBS_ID req, uint8_t cmd, ECN_FBS_ID *pk_res)
{
	mrb_state *mrb = svc->mrb;
	ER ret;
	ECN_FBS_ID res;

	ret = _ecn_fbs_cre(mrb, 1, &res);
	if (ret != E_OK) {
		ECN_DBG_PUT_2("_ecn_lcl_cre_res_fbs() : _ecn_fbs_cre() result = %d:%s", ret, itron_strerror(ret));
		return ret;
	}

	ret = _ecn_fbs_add_data(mrb, res, &cmd, sizeof(cmd));
	if (ret != E_OK) {
		_ecn_fbs_del(mrb, res);
		ECN_DBG_PUT_2("_ecn_lcl_cre_res_fbs() : _ecn_fbs_add_data() result = %d:%s", ret, itron_strerror(ret));
		return ret;
	}

	res.ptr->hdr.type = ECN_MSG_INTERNAL;
	res.ptr->hdr.sender.mbxid = &svc->lcl_mbxid;
	res.ptr->hdr.target.mbxid = req.ptr->hdr.reply.mbxid;
	res.ptr->hdr.reply.mbxid = &svc->lcl_mbxid;

	*pk_res = res;

	return E_OK;
}

/*
 *  内部メッセージ受信処理
 */
void _ecn_int_msg(ecnl_svc_task_t *svc, ECN_FBS_ID fbs_id, ECN_FBS_SSIZE_T a_snd_len)
{
	mrb_state *mrb = svc->mrb;
	ER result = E_OK, a_ret;
	ecn_lcl_msg_get_lcladdr_req_t msg;
	ecn_lcl_msg_get_lcladdr_error_t err;
	ECN_FBS_SSIZE_T len;
	ECN_FBS_ID buf;
	uint8_t cmd;
	mrb_value addr;

	a_ret = _ecn_fbs_get_data(mrb, fbs_id, &cmd, 1, &len);
	if (a_ret != E_OK) {
		ECN_DBG_PUT_2("[LCL TSK] _ecn_fbs_get_data() result = %d:%s", a_ret, itron_strerror(a_ret));
		return;
	}

	switch(cmd){
	// 通信レイヤーアドレスを返信
	case ECN_LCL_MSG_GET_LCLADDR_REQ:
		if (a_snd_len < sizeof(msg)) {
			result = E_PAR;
			break;
		}

		a_snd_len = 0;
		a_ret = _ecn_fbs_get_data(mrb, fbs_id, &msg, sizeof(msg), &a_snd_len);
		if (a_ret != E_OK) {
			ECN_DBG_PUT_2("[LCL TSK] _ecn_fbs_get_data() result = %d:%s", a_ret, itron_strerror(a_ret));
		}

		if (!lcl_is_valid_addrid(svc, msg.enodid)) {
			result = E_PAR;
			break;
		}
		
		a_ret = _ecn_lcl_cre_res_fbs(svc, fbs_id, ECN_LCL_MSG_GET_LCLADDR_RES, &buf);
		if (a_ret != E_OK) {
			return;
		}

		a_ret = _ecn_fbs_add_data_ex(mrb, buf, &msg.requestid, offsetof(ecn_lcl_msg_get_lcladdr_res_t, enodadrb));
		if (a_ret != E_OK) {
			_ecn_fbs_del(mrb, buf);
			ECN_DBG_PUT_2("_ecn_int_msg() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
			return;
		}

		addr = lcl_get_remote_addr(svc, msg.enodid);
		a_ret = _ecn_fbs_add_data_ex(mrb, buf, &addr, sizeof(addr));
		if (a_ret != E_OK) {
			_ecn_fbs_del(mrb, buf);
			ECN_DBG_PUT_2("_ecn_int_msg() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
			return;
		}

		a_ret = ecn_fbs_enqueue(buf.ptr->hdr.target.mbxid, buf.ptr);
		if (a_ret != E_OK) {
			_ecn_fbs_del(mrb, buf);
			ECN_DBG_PUT_2("_ecn_int_msg() : psnd_dtq() result = %d:%s", a_ret, itron_strerror(a_ret));
			return;
		}
		return;
	}

	a_ret = _ecn_lcl_cre_res_fbs(svc, fbs_id, ECN_LCL_MSG_GET_LCLADDR_ERROR, &buf);
	if (a_ret != E_OK) {
		return;
	}

	err.requestid = msg.requestid;
	err.error = result;
	a_ret = _ecn_fbs_add_data_ex(mrb, buf, &err, sizeof(err));
	if (a_ret != E_OK) {
		_ecn_fbs_del(mrb, buf);
		ECN_DBG_PUT_2("_ecn_int_msg() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
		return;
	}

	a_ret = ecn_fbs_enqueue(buf.ptr->hdr.target.mbxid, buf.ptr);
	if (a_ret != E_OK) {
		_ecn_fbs_del(mrb, buf);
		ECN_DBG_PUT_2("_ecn_int_msg() : psnd_dtq() result = %d:%s", a_ret, itron_strerror(a_ret));
		return;
	}
}

/*
 *  ECHONET 電文受信処理
 */
void _ecn_esv_msg(ecnl_svc_task_t *svc, ECN_FBS_ID fbs_id)
{
	mrb_state *mrb = svc->mrb;
	size_t a_len = _ecn_fbs_get_datalen(fbs_id);
	mrb_value a_msg;
	mrb_value a_dst;
	ER a_ret;
	ECN_FBS_SSIZE_T a_snd_len;

	/* 送信先通信レイヤーアドレス */
	a_dst = lcl_get_addr(svc, fbs_id.ptr->hdr.target.id);
	if (mrb_type(a_dst) != MRB_TT_FALSE/*nil*/) {
		ECN_DBG_PUT_4("[LCL TSK] echonet-node 0x%02X-0x%02X-0x%02X → lcl dest(%s)",
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx1,
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx2,
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx3,
			ip2str(NULL, &a_dst.lcladdr));

		a_msg = mrb_str_buf_new(mrb, a_len);

		/* fbsから出力領域にデータを抽出 */
		a_snd_len = 0;
		a_ret = _ecn_fbs_get_data(mrb, fbs_id, RSTRING_PTR(a_msg), a_len, &a_snd_len);
		if (a_ret != E_OK) {
			ECN_DBG_PUT_2("[UDP TSK] _ecn_fbs_get_data() result = %d:%s", a_ret, itron_strerror(a_ret));
		}
		else if (lcl_is_local_addr(svc, a_dst)) {
			RSTR_SET_LEN(RSTRING(a_msg), a_snd_len);

			/* 送信先が127.0.0.1 → mbxに転送 */
			ECN_DBG_PUT_1("redirect svc->lcl_mbxid → svc->svc_mbxid (esv:0x%02X)",
				((T_EDATA *)fbs_id.ptr)->hdr.edata.esv);

			a_ret = _ecn_lcl2mbx(svc, (const uint8_t *)RSTRING_PTR(a_msg), RSTRING_LEN(a_msg), a_dst);
			if (a_ret != E_OK) {
				ECN_DBG_PUT_2("_ecn_esv_msg() : _ecn_lcl2mbx() result = %d:%s", a_ret, itron_strerror(a_ret));
			}
		}
		else {
			RSTR_SET_LEN(RSTRING(a_msg), a_snd_len);

			ECN_DBG_PUT_2("[LCL TSK] lcl_snd_dat() to:%s %ubyte(s)",
				ip2str(NULL, &a_dst.lcladdr), a_snd_len);
#ifdef ECN_DBG_PUT_ENA
			_ecn_dbg_bindmp(RSTRING_PTR(a_msg), a_snd_len);
#endif
			/* 通信レイヤーへ送信 */
			a_ret = lcl_snd_msg(svc, a_dst, a_msg);
			if (a_ret < 0) {
				ECN_DBG_PUT_1("[LCL TSK] send, error: %s", itron_strerror(a_ret));
			}
		}

		/* データが長すぎて1パケットに収まらなかった場合 */
		if (_ecn_fbs_exist_data(fbs_id)) {
			ECN_DBG_PUT_1("[LCL TSK] send, data so long: %dbyte(s)", _ecn_fbs_get_datalen(fbs_id));
		}
	} else {
		ECN_DBG_PUT_3("[LCL TSK] echonet-node 0x%02X-0x%02X-0x%02X not found.",
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx1,
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx2,
			((T_EDATA *)fbs_id.ptr)->hdr.edata.deoj.eojx3);
	}
}

/*
 *  ノンブロッキングコールのコールバック関数
 */
ER echonet_lcl_input_msg(ecnl_svc_task_t *svc, mrb_value ep, mrb_value data)
{
	ECHONET_LCL_SEND_TO_MBX;

	_ecn_lcl2mbx(svc, (const uint8_t *)RSTRING_PTR(data), RSTRING_LEN(data), ep);

	return E_OK;
}

/*
 *  リモートECHONETノードの適合確認
 */
bool_t ecn_is_match(ecnl_svc_task_t *svc, const EOBJCB *enodcb, T_EDATA *edata, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	ER			ret;
	T_ENUM_EPC	enm;
	uint8_t		epc;
	uint8_t		pdc;
	uint8_t		p_edt[256];
	int			i, j, k;
	int			count;
	const EOBJINIB	*p_eobj;
	bool_t		match;

	if (!edata)
		return false;

	/* ノードスタート時インスタンスリスト通知以外は除外 */
	if (	edata->hdr.edata.esv != ESV_INF
		||	edata->hdr.edata.deoj.eojx1 != EOJ_X1_PROFILE
		||	edata->hdr.edata.deoj.eojx2 != EOJ_X2_NODE_PROFILE
		||	edata->hdr.edata.deoj.eojx3 != 0x01
		||	edata->hdr.edata.seoj.eojx1 != EOJ_X1_PROFILE
		||	edata->hdr.edata.seoj.eojx2 != EOJ_X2_NODE_PROFILE
		||	(	edata->hdr.edata.seoj.eojx3 != 0x01
			&&	edata->hdr.edata.seoj.eojx3 != 0x02)) {
		return false;
	}

	ret = ecn_itr_ini(&enm, edata);
	if (ret) {
		ECN_DBG_PUT_2("ecn_is_match(): ecn_itr_ini() result = %d:%s", ret, itron_strerror(ret));
		return false;
	}
	while ((ret = ecn_itr_nxt(mrb, &enm, &epc, &pdc, p_edt)) == E_OK) {
		if (enm.is_eof) {
			break;
		}
		ECN_DBG_PUT_2("ecn_is_match(): ecn_itr_nxt() result: epc=0x%02X, pdc=%d", epc, pdc);
		/* インスタンスリスト通知または自ノードインスタンスリストＳ以外は除外 */
		if ((epc != 0xD5) && (epc != 0xD6)) {
			continue;
		}

		/* ２バイト目以降にeojが列挙されている */
		count = (pdc - 1) / sizeof(T_ECN_EOJ);

		/* ノード内の機器オブジェクトを検索 */
		for (k = 0; k < enodcb->eobjcnt; k++) {
			p_eobj = enodcb->eobjs[k];

			/* インスタンスリストを確認 */
			match = false;
			for (i = 0, j = 1; i < count; i++, j += sizeof(T_ECN_EOJ)) {
				if (p_eobj->eojx1 != p_edt[j])
					continue;
				if (p_eobj->eojx2 != p_edt[j + 1])
					continue;
				if (p_eobj->eojx3 != p_edt[j + 2])
					continue;

				match = true;
				break;
			}

			if (!match)
				return false;
		}

		/* すべて揃っていたら適合（インスタンスリストの方が多くてもいいこととする） */
		return true;
	}

	return false;
}

/*
 *  通信レイヤーアドレスからリモートECHONETノードへ変換
 */
ECN_ENOD_ID lcl_get_id(ecnl_svc_task_t *svc, T_EDATA *edata, const mrb_value ep)
{
	ECN_ENOD_ID id;

	if (lcl_is_local_addr(svc, ep))
		return ENOD_LOCAL_ID;
	if (lcl_is_multicast_addr(svc, ep))
		return ENOD_MULTICAST_ID;

	/* 通信レイヤーアドレスの同じものを検索 */
	if ((id = lcl_get_remote_id(svc, ep)) != ENOD_NOT_MATCH_ID) {
		ECN_CAP_PUT_2("lcl_get_id(): ip-found remote(%d) = %s",
			id - ENOD_REMOTE_ID, ip2str(NULL, &lcladdr));
		return id;
	}

	/* 対応するリモートノードがあれば通信レイヤーアドレスを設定 */
	if ((id = lcl_set_remote_addr(svc, edata, ep)) != ENOD_NOT_MATCH_ID) {
		ECN_CAP_PUT_2("lcl_get_id(): enod-found remote(%d) = %s",
			id - ENOD_REMOTE_ID, ip2str(NULL, &lcladdr));
		return id;
	}

	/* 空き領域を探して自動登録 */
	if ((id = lcl_add_remote_addr(svc, edata, ep)) != ENOD_NOT_MATCH_ID) {
		ECN_CAP_PUT_2("lcl_get_id(): empty-found remote(%d) = %s",
			id - ENOD_REMOTE_ID, ip2str(NULL, &lcladdr));
		return id;
	}

	return ENOD_NOT_MATCH_ID;
}

/*
 *  リモートECHONETノードから通信レイヤーアドレスへ変換
 */
mrb_value lcl_get_addr(ecnl_svc_task_t *svc, ECN_ENOD_ID fa_enodid)
{
	if (fa_enodid == ENOD_MULTICAST_ID) {
		/* targetがENOD_MULTICAST_IDの場合、マルチキャストを行う */
		return lcl_get_multicast_addr(svc);
	}

	if (fa_enodid < ENOD_REMOTE_ID) {
		/* targetが未定義・LOCAL・APIの場合、ローカル配送を行う */
		return lcl_get_local_addr(svc);
	}

	return lcl_get_remote_addr(svc, fa_enodid);
}

ER ecn_lcl_get_lcladdr(ecnl_svc_task_t *svc, T_ECN_FBS_QUEUE *sender, int requestid, ECN_ENOD_ID enodid, ECN_FBS_ID *pk_req)
{
	mrb_state *mrb = svc->mrb;
	ER a_ret;
	ECN_FBS_ID req;
	mrb_value addr;

	a_ret = _ecn_lcl_cre_req_fbs(svc, sender, ECN_LCL_MSG_GET_LCLADDR_REQ, &req);
	if (a_ret != E_OK) {
		return a_ret;
	}

	a_ret = _ecn_fbs_add_data_ex(mrb, req, &requestid, sizeof(((ecn_lcl_msg_get_lcladdr_res_t *)0)->requestid));
	if (a_ret != E_OK) {
		_ecn_fbs_del(mrb, req);
		ECN_DBG_PUT_2("ecn_lcl_get_lcladdr() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
		return a_ret;
	}

	a_ret = _ecn_fbs_add_data_ex(mrb, req, &enodid, sizeof(((ecn_lcl_msg_get_lcladdr_res_t *)0)->enodid));
	if (a_ret != E_OK) {
		_ecn_fbs_del(mrb, req);
		ECN_DBG_PUT_2("ecn_lcl_get_lcladdr() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
		return a_ret;
	}

	addr = lcl_get_remote_addr(svc, enodid);
	a_ret = _ecn_fbs_add_data_ex(mrb, req, &addr, sizeof(addr));
	if (a_ret != E_OK) {
		_ecn_fbs_del(mrb, req);
		ECN_DBG_PUT_2("ecn_lcl_get_lcladdr() : _ecn_fbs_add_data_ex() result = %d:%s", a_ret, itron_strerror(a_ret));
		return a_ret;
	}

	*pk_req = req;

	return E_OK;
}
