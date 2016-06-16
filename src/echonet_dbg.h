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
 *		ECHONET Lite タスク デバッグ出力
 */

#ifndef TOPPERS_ECHONET_DBG_H
#define TOPPERS_ECHONET_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "echonet.h"

#ifdef ECN_DBG_PUT_ENA
#define ECN_DBG_PUT(a)					printf(a)
#define ECN_DBG_PUT_1(a, b)				printf(a, b)
#define ECN_DBG_PUT_2(a, b, c)			printf(a, b, c)
#define ECN_DBG_PUT_3(a, b, c, d)		printf(a, b, c, d)
#define ECN_DBG_PUT_4(a, b, c, d, e)	printf(a, b, c, d, e)
#define ECN_DBG_PUT_5(a, b, c, d, e, f)	printf(a, b, c, d, e, f)
#else
#define ECN_DBG_PUT(a)
#define ECN_DBG_PUT_1(a, b)
#define ECN_DBG_PUT_2(a, b, c)
#define ECN_DBG_PUT_3(a, b, c, d)
#define ECN_DBG_PUT_4(a, b, c, d, e)
#define ECN_DBG_PUT_5(a, b, c, d, e, f)
#endif

/* デバッグ出力(CARP) */
#ifdef ECN_CAP_PUT_ENA
#define ECN_CAP_PUT(a)					printf(a)
#define ECN_CAP_PUT_1(a, b)				printf(a, b)
#define ECN_CAP_PUT_2(a, b, c)			printf(a, b, c)
#define ECN_CAP_PUT_3(a, b, c, d)		printf(a, b, c, d)
#define ECN_CAP_PUT_4(a, b, c, d, e)	printf(a, b, c, d, e)
#define ECN_CAP_PUT_5(a, b, c, d, e, f)	printf(a, b, c, d, e, f)
#else
#define ECN_CAP_PUT(a)
#define ECN_CAP_PUT_1(a, b)
#define ECN_CAP_PUT_2(a, b, c)
#define ECN_CAP_PUT_3(a, b, c, d)
#define ECN_CAP_PUT_4(a, b, c, d, e)
#define ECN_CAP_PUT_5(a, b, c, d, e, f)
#endif

#if !defined(ECN_DBG_PUT_ENA) && !defined(ECN_CAP_PUT_ENA)
#define itron_strerror(errorno) #errorno
#else
const char *_ecn_dbg_enod2str(ECN_ENOD_ID fa_enod_id);
void _ecn_dbg_bindmp(const uint8_t *buffer, size_t len);
const char *_ecn_dbg_esv2str(uint8_t fa_esv);
const char *itron_strerror(int errorno);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TOPPERS_ECHONET_DBG_H */
