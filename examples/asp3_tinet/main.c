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
 *  サンプルプログラム(1)の本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "main.h"
#include "rza1.h"
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/dump.h>
#include <mruby/string.h>
#include <tinet_config.h>
#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <tinet_nic_defs.h>
#include <tinet_cfg.h>
#include <netinet/in4_var.h>
#include <net/ethernet.h>
#include <net/if6_var.h>
#include <ethernet_api.h>

uint8_t mac_addr[6] = { 0x00, 0x30, 0x13, 0x06, 0x62, 0xC0 };

struct udp_msg {
	T_IPV4EP dst;
	int len;
	uint8_t buffer[ETHER_MAX_LEN];
};

static void netif_link_callback(T_IFNET *ether);
static void main_initialize();
static void main_finalize();
static void main_ntf_inl();
static TMO main_get_timer();
static void main_progress(TMO interval);
static void main_recieve_msg(struct udp_msg *msg);
static void main_release_msg(struct udp_msg *msg);
static void main_timeout();

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	ER ret, ret2;
	SYSTIM prev, now;
	TMO timer;
	struct udp_msg *msg;

	/* MACアドレスを設定 */
	ethernet_address(mac_addr);

	/* TINETが起動するまで待つ */
	ether_set_link_callback(netif_link_callback);

	/* 初期化 */
	main_initialize();

	ret2 = get_tim(&now);
	if (ret2 != E_OK){
		syslog(LOG_ERROR, "get_tim");
		return;
	}

	for(;;){
		prev = now;

		/* タイマー取得 */
		timer = main_get_timer();

		/* メッセージ待ち */
		ret = trcv_dtq(MAIN_DATAQUEUE, (intptr_t *)&msg, timer);
		if ((ret != E_OK) && (ret != E_TMOUT)){
			syslog(LOG_ERROR, "trcv_dtq");
			break;
		}

		ret2 = get_tim(&now);
		if (ret2 != E_OK){
			syslog(LOG_ERROR, "get_tim");
			break;
		}

		/* 時間経過 */
		main_progress(now - prev);

		/* Echonet電文受信の場合 */
		if (ret == E_OK) {
			if (msg->dst.ipaddr == 0){
				/* Ethernet Link up */
				if (msg->buffer[0] & IF_FLAG_LINK_UP) {
					/* DHCP開始 */
				}
				else if (msg->buffer[0] & IF_FLAG_UP) {
					/* 初期通知 */
					main_ntf_inl();
				}
			}
			else{
				/* Echonet電文受信処理 */
				main_recieve_msg(msg);

				/* 領域解放 */
				main_release_msg(msg);
			}
		}

		/* タイムアウト処理 */
		main_timeout();
	}

	main_finalize();
}

ER callback_nblk_udp(ID cepid, FN fncd, void *p_parblk)
{
	static struct udp_msg msg_inst[2];
	static int msg_no = 0;
	struct udp_msg *msg = &msg_inst[msg_no];
	ER	error = E_OK;

	switch (fncd) {
	case TFN_UDP_CRE_CEP:
	case TFN_UDP_RCV_DAT:
		/* ECN_CAP_PUT("[UDP ECHO SRV] callback_nblk_udp() recv: %u", *(int *)p_parblk); */
		memset(msg, 0, sizeof(struct udp_msg));
		if ((msg->len = udp_rcv_dat(cepid, &msg->dst, msg->buffer, sizeof(msg->buffer), 0)) < 0) {
			syslog(LOG_WARNING, "[UDP ECHO SRV] recv, error: %s", itron_strerror(msg->len));
			return msg->len;
		}
		msg_no = (msg_no + 1) % 2;
		return snd_dtq(MAIN_DATAQUEUE, (intptr_t)msg);

	case TFN_UDP_SND_DAT:
		break;
	default:
		syslog(LOG_WARNING, "[UDP ECHO SRV] fncd:0x%04X(%s)", -fncd,
			(fncd == TFN_UDP_CRE_CEP ? "TFN_UDP_CRE_CEP" :
			(fncd == TFN_UDP_RCV_DAT ? "TFN_UDP_RCV_DAT" :
			(fncd == TFN_UDP_SND_DAT ? "TFN_UDP_SND_DAT" : "undef"))));

		error = E_PAR;
		break;
	}
	return error;
}

static void netif_link_callback(T_IFNET *ether) {
	static struct udp_msg msg_inst[2];
	static int msg_no = 0;
	struct udp_msg *msg = &msg_inst[msg_no];

	memset(msg, 0, sizeof(struct udp_msg));

	msg->len = 1;
	msg->buffer[0] = ether->_flags;

	msg_no = (msg_no + 1) % 2;
	return snd_dtq(MAIN_DATAQUEUE, (intptr_t)msg);
}

mrb_state *mrb;
struct RClass *_module_sample;
struct RClass *_class_main;
mrb_value main_obj;
void mrb_mruby_others_gem_init(mrb_state* mrb);
void mrb_mruby_others_gem_final(mrb_state* mrb);

extern const uint8_t main_rb_code[];

static void mruby_init()
{
}

/*
 * 初期化
 */
static void main_initialize()
{
	struct RProc* n;
	struct mrb_irep *irep;

	/* mrubyの初期化 */
	mrb = mrb_open();
	if (mrb == NULL)
		abort();

	irep = mrb_read_irep(mrb, main_rb_code);
	n = mrb_proc_new(mrb, irep);
	mrb_run(mrb, n, mrb_nil_value());
}

static void main_finalize()
{
	mrb_close(mrb);
}

/* インスタンスリスト通知の送信 */
static void main_ntf_inl()
{
	mrb_funcall(mrb, main_obj, "ntf_inl", 0);
}

/*
 * タイマー取得
 */
static TMO main_get_timer()
{
	mrb_value result;

	result = mrb_funcall(mrb, main_obj, "get_timer", 0);
	if(mrb_fixnum_p(result))
		return mrb_fixnum(result);

	return TMO_FEVR;
}

/*
 * 時間経過
 */
static void main_progress(TMO interval)
{
	mrb_funcall(mrb, main_obj, "progress", 1, mrb_fixnum_value(interval));
}

/*
 * Echonet電文受信処理
 */
static void main_recieve_msg(struct udp_msg *msg)
{
	mrb_int maxlen, flags = 0;
	mrb_value ep, buf;

	/* 通信端点 */
	ep = mrb_str_new(mrb, (char *)&msg->dst, sizeof(msg->dst));

	/* 受信データ */
	buf = mrb_str_new(mrb, (char *)msg->buffer, msg->len);

	/* 通信レイヤーからの入力 */
	mrb_funcall(mrb, main_obj, "recv_msg", 2, ep, buf);
}

/*
 * メッセージ破棄
 */
static void main_release_msg(struct udp_msg *msg)
{
}

static void main_timeout()
{
	mrb_funcall(mrb, main_obj, "timeout", 0);
}

static mrb_value mrb_sample_main_set(mrb_state *mrb, mrb_value self)
{
	mrb_get_args(mrb, "o", &main_obj);

	return self;
}

static mrb_value mrb_sample_main_set_led(mrb_state *mrb, mrb_value self)
{
	mrb_value value;

	mrb_get_args(mrb, "i", &value);

	sil_wrb_mem((uint8_t *)0xFFFFF412, mrb_fixnum(value));

	return self;
}

static mrb_value mrb_sample_main_get_led(mrb_state *mrb, mrb_value self)
{
	uint8_t value;

	value = sil_reb_mem((uint8_t *)0xFFFFF412);

	return mrb_fixnum_value(value);
}

static mrb_value mrb_sample_main_get_btn(mrb_state *mrb, mrb_value self)
{
	bool_t value;
	mrb_value no;

	mrb_get_args(mrb, "i", &no);

	switch(mrb_fixnum(no)){
	case 1:
		value = (sil_reb_mem((uint8_t *)0xFFFFF413) & 0x01) != 0;
		break;
	case 2:
		value = (sil_reb_mem((uint8_t *)0xFFFFF413) & 0x10) != 0;
		break;
	default:
		value = false;
		break;
	}

	return value ? mrb_true_value() : mrb_false_value();
}

static mrb_value mrb_sample_main_get_dsw(mrb_state *mrb, mrb_value self)
{
	uint8_t value;

	value = (sil_reb_mem((uint8_t *)0xFFFFF40F) & 0x01) != 0;

	return mrb_fixnum_value(value);
}

static mrb_value mrb_sample_main_snd_msg(mrb_state *mrb, mrb_value self)
{
	mrb_value rep;
	mrb_value rdat;
	T_IPV4EP *ep;
	ER_UINT ret;

	mrb_get_args(mrb, "SS", &rep, &rdat);

	if (RSTRING_LEN(rep) != sizeof(T_IPV4EP)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "snd_msg");
		return mrb_nil_value();
	}

	ep = (T_IPV4EP *)RSTRING_PTR(rep);

	ret = udp_snd_dat(MAIN_ECNL_UDP_CEPID, ep, RSTRING_PTR(rdat), RSTRING_LEN(rdat), 1000);
	if (ret < 0) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "snd_msg");
		return mrb_nil_value();
	}

	return mrb_nil_value();
}

static mrb_value mrb_sample_main_is_local_addr(mrb_state *mrb, mrb_value self)
{
	mrb_value rep;
	T_IPV4EP *ep;

	mrb_get_args(mrb, "S", &rep);

	if (RSTRING_LEN(rep) < sizeof(T_IPV4EP)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "is_local_addr");
		return mrb_nil_value();
	}

	ep = (T_IPV4EP *)RSTRING_PTR(rep);

	return (ep->ipaddr == MAKE_IPV4_ADDR(127, 0, 0, 1)) ? mrb_true_value() : mrb_false_value();
}

static mrb_value mrb_sample_main_is_multicast_addr(mrb_state *mrb, mrb_value self)
{
	mrb_value rep;
	T_IPV4EP *ep;

	mrb_get_args(mrb, "S", &rep);

	if (RSTRING_LEN(rep) < sizeof(T_IPV4EP)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "is_multicast_addr");
		return mrb_nil_value();
	}

	ep = (T_IPV4EP *)RSTRING_PTR(rep);

	return (ep->ipaddr == MAKE_IPV4_ADDR(224, 0, 23, 0)) ? mrb_true_value() : mrb_false_value();
}

static mrb_value mrb_sample_main_equals_addr(mrb_state *mrb, mrb_value self)
{
	mrb_value rep1, rep2;
	T_IPV4EP *ep1, *ep2;

	mrb_get_args(mrb, "SS", &rep1, &rep2);

	if ((RSTRING_LEN(rep1) != sizeof(T_IPV4EP)) || (RSTRING_LEN(rep2) != sizeof(T_IPV4EP))) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "equals_addr");
		return mrb_nil_value();
	}

	ep1 = (T_IPV4EP *)RSTRING_PTR(rep1);
	ep2 = (T_IPV4EP *)RSTRING_PTR(rep2);

	return (ep1->ipaddr == ep2->ipaddr) ? mrb_true_value() : mrb_false_value();
}

static mrb_value mrb_sample_main_get_local_addr(mrb_state *mrb, mrb_value self)
{
	T_IPV4EP ep;
	mrb_value rep;

	ep.ipaddr = MAKE_IPV4_ADDR(127, 0, 0, 1);
	ep.portno = 3610;

	rep = mrb_str_new(mrb, (char *)&ep, sizeof(ep));

	return rep;
}

static mrb_value mrb_sample_main_get_multicast_addr(mrb_state *mrb, mrb_value self)
{
	T_IPV4EP ep;
	mrb_value rep;

	ep.ipaddr = MAKE_IPV4_ADDR(224, 0, 23, 0);
	ep.portno = 3610;

	rep = mrb_str_new(mrb, (char *)&ep, sizeof(ep));

	return rep;
}

void mrb_mruby_others_gem_init(mrb_state* mrb)
{
	_module_sample = mrb_define_module(mrb, "TargetBoard");

	mrb_define_class_method(mrb, _module_sample, "set", mrb_sample_main_set, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_sample, "set_led", mrb_sample_main_set_led, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_sample, "get_led", mrb_sample_main_get_led, MRB_ARGS_NONE());
	mrb_define_class_method(mrb, _module_sample, "get_btn", mrb_sample_main_get_btn, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_sample, "get_dsw", mrb_sample_main_get_dsw, MRB_ARGS_NONE());
	mrb_define_class_method(mrb, _module_sample, "snd_msg", mrb_sample_main_snd_msg, MRB_ARGS_REQ(2));
	mrb_define_class_method(mrb, _module_sample, "is_local_addr", mrb_sample_main_is_local_addr, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_sample, "is_multicast_addr", mrb_sample_main_is_multicast_addr, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_sample, "equals_addr", mrb_sample_main_equals_addr, MRB_ARGS_REQ(2));
	mrb_define_class_method(mrb, _module_sample, "get_local_addr", mrb_sample_main_get_local_addr, MRB_ARGS_NONE());
	mrb_define_class_method(mrb, _module_sample, "get_multicast_addr", mrb_sample_main_get_multicast_addr, MRB_ARGS_NONE());
}

void mrb_mruby_others_gem_final(mrb_state* mrb)
{
}
