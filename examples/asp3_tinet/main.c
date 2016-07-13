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
#include <stdlib.h>
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
#include <netinet/in_var.h>
#include <net/ethernet.h>
#include <net/if6_var.h>
#include <net/net.h>
#include <net/if_var.h>
#include <netinet/udp_var.h>
#include <ethernet_api.h>
#include "mruby_arduino.h"

uint8_t mac_addr[6] = { 0x00, 0x30, 0x13, 0x06, 0x62, 0xC0 };

struct udp_msg {
	T_IPV4EP dst;
	int len;
	uint8_t buffer[ETHER_MAX_LEN];
};

SYSTIM main_time;
mrb_state *mrb;
struct RClass *_module_target_board;
static void netif_link_callback(T_IFNET *ether);

extern const uint8_t main_rb_code[];

/* MACアドレスの設定時に呼ばれる */
void mbed_mac_address(char *mac) {
	memcpy(mac, mac_addr, 6);
}

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	ER ret;
	struct RProc* n;
	struct mrb_irep *irep;

	/* mruby_arduino初期化 */
	mruby_arduino_init();

	/* TINETが起動するまで待つ */
	ether_set_link_callback(netif_link_callback);

	/* mrubyの初期化 */
	mrb = mrb_open();
	if (mrb == NULL)
		abort();

	ret = get_tim(&main_time);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "get_tim");
		return;
	}

	irep = mrb_read_irep(mrb, main_rb_code);
	n = mrb_proc_new(mrb, irep);
	mrb_run(mrb, n, mrb_nil_value());

	mrb_close(mrb);
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
	msg->buffer[0] = ether->flags;

	msg_no = (msg_no + 1) % 2;
	snd_dtq(MAIN_DATAQUEUE, (intptr_t)msg);
}

/*
 * アプリケーションタスクの登録
 */
static mrb_value mrb_target_board_wait_msg(mrb_state *mrb, mrb_value self)
{
	TMO timer;
	SYSTIM now;
	ER ret, ret2;
	struct udp_msg *msg;
	mrb_value arv[3];

	mrb_get_args(mrb, "i", &timer);
	if (timer != TMO_FEVR)
		timer *= 1000;

	/* メッセージ待ち */
	ret = trcv_dtq(MAIN_DATAQUEUE, (intptr_t *)&msg, timer);
	if ((ret != E_OK) && (ret != E_TMOUT)) {
		syslog(LOG_ERROR, "trcv_dtq");
		return mrb_nil_value();
	}

	ret2 = get_tim(&now);
	if (ret2 != E_OK) {
		syslog(LOG_ERROR, "get_tim");
		return mrb_nil_value();
	}

	arv[0] = mrb_fixnum_value((now - main_time) / 1000);
	main_time = now;

	/* タイムアウトの場合 */
	if (ret == E_TMOUT) {
		return mrb_ary_new_from_values(mrb, 1, arv);
	}

	/* 内部イベントの場合 */
	if (msg->dst.ipaddr == 0) {
		/* Ethernet Link up */
		if (msg->buffer[0] & IF_FLAG_LINK_UP) {
			arv[1] = mrb_fixnum_value(1);
		}
		/* EP Link up */
		else if (msg->buffer[0] & IF_FLAG_UP) {
			arv[1] = mrb_fixnum_value(2);
		}
		else {
			arv[1] = mrb_fixnum_value(0);
		}

		return mrb_ary_new_from_values(mrb, 2, arv);
	}
	/* Echonet電文受信の場合 */
	else {
		/* 通信端点 */
		arv[1] = mrb_str_new(mrb, (char *)&msg->dst, sizeof(msg->dst));

		/* 受信データ */
		arv[2] = mrb_str_new(mrb, (char *)msg->buffer, msg->len);

		return mrb_ary_new_from_values(mrb, 3, arv);
	}
}

/*
 * アプリケーションタスクの登録
 */
static mrb_value mrb_target_board_restart(mrb_state *mrb, mrb_value self)
{
	/* DHCP開始 */

	return self;
}

/*
 * 通信レイヤーへの送信
 */
static mrb_value mrb_target_board_snd_msg(mrb_state *mrb, mrb_value self)
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

	ret = udp_snd_dat(MAIN_ECNL_UDP_CEPID, ep, RSTRING_PTR(rdat), RSTRING_LEN(rdat), TMO_FEVR);
	if (ret < 0) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "snd_msg");
		return mrb_nil_value();
	}

	return mrb_nil_value();
}

/*
 * ローカルアドレスか確認
 */
static mrb_value mrb_target_board_is_local_addr(mrb_state *mrb, mrb_value self)
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

/*
 * マルチキャストアドレスか確認
 */
static mrb_value mrb_target_board_is_multicast_addr(mrb_state *mrb, mrb_value self)
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

/*
 * 同一アドレスか確認
 */
static mrb_value mrb_target_board_equals_addr(mrb_state *mrb, mrb_value self)
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

/*
 * ローカルアドレスの取得
 */
static mrb_value mrb_target_board_get_local_addr(mrb_state *mrb, mrb_value self)
{
	T_IPV4EP ep;
	mrb_value rep;

	ep.ipaddr = MAKE_IPV4_ADDR(127, 0, 0, 1);
	ep.portno = 3610;

	rep = mrb_str_new(mrb, (char *)&ep, sizeof(ep));

	return rep;
}

/*
 * マルチキャストアドレスの取得
 */
static mrb_value mrb_target_board_get_multicast_addr(mrb_state *mrb, mrb_value self)
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
	_module_target_board = mrb_define_module(mrb, "TargetBoard");

	// mbed Pin Names
	mrb_define_const(mrb, _module_target_board, "LED1", mrb_fixnum_value(LED1));
	mrb_define_const(mrb, _module_target_board, "LED2", mrb_fixnum_value(LED2));
	mrb_define_const(mrb, _module_target_board, "LED3", mrb_fixnum_value(LED3));
	mrb_define_const(mrb, _module_target_board, "LED4", mrb_fixnum_value(LED4));

	mrb_define_const(mrb, _module_target_board, "LED_RED", mrb_fixnum_value(LED_RED));
	mrb_define_const(mrb, _module_target_board, "LED_GREEN", mrb_fixnum_value(LED_GREEN));
	mrb_define_const(mrb, _module_target_board, "LED_BLUE", mrb_fixnum_value(LED_BLUE));
	mrb_define_const(mrb, _module_target_board, "LED_USER", mrb_fixnum_value(LED_USER));

	mrb_define_const(mrb, _module_target_board, "USBTX", mrb_fixnum_value(USBTX));
	mrb_define_const(mrb, _module_target_board, "USBRX", mrb_fixnum_value(USBRX));

	// Arduiono Pin Names
	mrb_define_const(mrb, _module_target_board, "D0", mrb_fixnum_value(D0));
	mrb_define_const(mrb, _module_target_board, "D1", mrb_fixnum_value(D1));
	mrb_define_const(mrb, _module_target_board, "D2", mrb_fixnum_value(D2));
	mrb_define_const(mrb, _module_target_board, "D3", mrb_fixnum_value(D3));
	mrb_define_const(mrb, _module_target_board, "D4", mrb_fixnum_value(D4));
	mrb_define_const(mrb, _module_target_board, "D5", mrb_fixnum_value(D5));
	mrb_define_const(mrb, _module_target_board, "D6", mrb_fixnum_value(D6));
	mrb_define_const(mrb, _module_target_board, "D7", mrb_fixnum_value(D7));
	mrb_define_const(mrb, _module_target_board, "D8", mrb_fixnum_value(D8));
	mrb_define_const(mrb, _module_target_board, "D9", mrb_fixnum_value(D9));
	mrb_define_const(mrb, _module_target_board, "D10", mrb_fixnum_value(D10));
	mrb_define_const(mrb, _module_target_board, "D11", mrb_fixnum_value(D11));
	mrb_define_const(mrb, _module_target_board, "D12", mrb_fixnum_value(D12));
	mrb_define_const(mrb, _module_target_board, "D13", mrb_fixnum_value(D13));
	mrb_define_const(mrb, _module_target_board, "D14", mrb_fixnum_value(D14));
	mrb_define_const(mrb, _module_target_board, "D15", mrb_fixnum_value(D15));

	mrb_define_const(mrb, _module_target_board, "A0", mrb_fixnum_value(A0));
	mrb_define_const(mrb, _module_target_board, "A1", mrb_fixnum_value(A1));
	mrb_define_const(mrb, _module_target_board, "A2", mrb_fixnum_value(A2));
	mrb_define_const(mrb, _module_target_board, "A3", mrb_fixnum_value(A3));
	mrb_define_const(mrb, _module_target_board, "A4", mrb_fixnum_value(A4));
	mrb_define_const(mrb, _module_target_board, "A5", mrb_fixnum_value(A5));

	mrb_define_const(mrb, _module_target_board, "I2C_SCL", mrb_fixnum_value(I2C_SCL));
	mrb_define_const(mrb, _module_target_board, "I2C_SDA", mrb_fixnum_value(I2C_SDA));

	mrb_define_const(mrb, _module_target_board, "USER_BUTTON0", mrb_fixnum_value(USER_BUTTON0));

	// Not connected
	mrb_define_const(mrb, _module_target_board, "NC", mrb_fixnum_value(NC));

	mrb_define_class_method(mrb, _module_target_board, "wait_msg", mrb_target_board_wait_msg, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_target_board, "restart", mrb_target_board_restart, MRB_ARGS_NONE());
	mrb_define_class_method(mrb, _module_target_board, "snd_msg", mrb_target_board_snd_msg, MRB_ARGS_REQ(2));
	mrb_define_class_method(mrb, _module_target_board, "is_local_addr", mrb_target_board_is_local_addr, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_target_board, "is_multicast_addr", mrb_target_board_is_multicast_addr, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, _module_target_board, "equals_addr", mrb_target_board_equals_addr, MRB_ARGS_REQ(2));
	mrb_define_class_method(mrb, _module_target_board, "get_local_addr", mrb_target_board_get_local_addr, MRB_ARGS_NONE());
	mrb_define_class_method(mrb, _module_target_board, "get_multicast_addr", mrb_target_board_get_multicast_addr, MRB_ARGS_NONE());
}

void mrb_mruby_others_gem_final(mrb_state* mrb)
{
}
