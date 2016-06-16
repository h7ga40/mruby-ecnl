/*
 *  TOPPERS ECHONET Lite Communication Middleware
 * 
 *  Copyright (C) 2016 Cores Co., Ltd. Japan
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include "echonet.h"
#include "echonet_fbs.h"
#include "echonet_agent.h"
#include "echonet_task.h"
#include "echonet_lcl_task.h"
#include "mrb_ecnl.h"

struct RClass *_module_ecnl;
struct RClass *_class_object;
struct RClass *_class_node;
struct RClass *_class_property;
struct RClass *_class_data;
struct RClass *_class_iterator;
struct RClass *_class_svctask;

static void mrb_ecnl_eobject_free(mrb_state *mrb, void *ptr);
static void mrb_ecnl_enode_free(mrb_state *mrb, void *ptr);
static void mrb_ecnl_eproperty_free(mrb_state *mrb, void *ptr);
static void mrb_ecnl_edata_free(mrb_state *mrb, void *ptr);
static void mrb_ecnl_eiterator_free(mrb_state *mrb, void *ptr);
static void mrb_ecnl_svctask_free(mrb_state *mrb, void *ptr);

const static struct mrb_data_type mrb_ecnl_eobject_type = {"EObject", mrb_ecnl_eobject_free};
const static struct mrb_data_type mrb_ecnl_enode_type = {"ENode", mrb_ecnl_enode_free};
const static struct mrb_data_type mrb_ecnl_eproperty_type = {"EProperty", mrb_ecnl_eproperty_free};
const static struct mrb_data_type mrb_ecnl_edata_type = {"EData", mrb_ecnl_edata_free};
const static struct mrb_data_type mrb_ecnl_eiterator_type = {"EIterator", mrb_ecnl_eiterator_free};
const static struct mrb_data_type mrb_ecnl_svctask_type = {"SvcTask", mrb_ecnl_svctask_free};

static mrb_value mrb_ecnl_eobject_initialize(mrb_state *mrb, mrb_value self)
{
	ecn_device_t *obj;
	mrb_value node;
	mrb_int eojx1;
	mrb_int eojx2;
	mrb_int eojx3;
	mrb_value props;
	const mrb_value *rprop;
	const EPRPINIB **eprp;
	int i, count;

	mrb_get_args(mrb, "iiioA", &eojx1, &eojx2, &eojx3, &node, &props);

	if (!mrb_obj_is_kind_of(mrb, node, _class_node)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "EObject.new");
		return mrb_nil_value();
	}

	rprop = RARRAY_PTR(props);
	count = RARRAY_LEN(props);

	obj = (ecn_device_t *)mrb_malloc(mrb, sizeof(ecn_device_t) + count * sizeof(EPRPINIB **));
	DATA_TYPE(self) = &mrb_ecnl_eobject_type;
	DATA_PTR(self) = obj;

	eprp = (const EPRPINIB **)&obj[1];

	obj->base.inib.eobjatr = EOBJ_DEVICE;
	obj->base.inib.enodid = 0;
	obj->base.inib.exinf = (intptr_t)NULL;
	obj->base.inib.eojx1 = eojx1;
	obj->base.inib.eojx2 = eojx2;
	obj->base.inib.eojx3 = eojx3;
	obj->base.inib.eprp = eprp;

	obj->node = (ecn_node_t *)DATA_PTR(node);

	for (i = 0; i < count; i++) {
		T_MRB_ECNL_EPROPERTY *prop;

		if (!mrb_obj_is_kind_of(mrb, rprop[i], _class_property)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "eprpinib_table");
			goto error;
		}

		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		prop->eobj = self;
		eprp[i] = &prop->inib;
	}

	return self;
error:
	self = mrb_nil_value();
	mrb_free(mrb, obj);

	return self;
}

static void mrb_ecnl_eobject_free(mrb_state *mrb, void *ptr)
{
	ecn_device_t *obj = (ecn_device_t *)ptr;

	/* 機器オブジェクトの設定として取り込まれた場合は破棄しない */
	if(obj->base.svc == NULL)
		mrb_free(mrb, obj);
}

static mrb_value mrb_ecnl_eobject_data_prop_set(mrb_state *mrb, mrb_value self)
{
	mrb_value rprp;
	mrb_value rdat;
	mrb_value data;
	T_MRB_ECNL_EPROPERTY *prop;

	mrb_get_args(mrb, "oS", &rprp, &rdat);

	if (!mrb_obj_is_kind_of(mrb, rprp, _class_property)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "data_prop_set");
		return mrb_fixnum_value(0);
	}

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprp);
	data = mrb_iv_get(mrb, self, prop->exinf);

	prop->anno = !mrb_str_equal(mrb, data, rdat);

	mrb_iv_set(mrb, self, prop->exinf, rdat);

	return mrb_fixnum_value(RSTRING_LEN(rdat));
}

static mrb_value mrb_ecnl_eobject_data_prop_get(mrb_state *mrb, mrb_value self)
{
	mrb_value rprp;
	mrb_value rsiz;
	mrb_value rdat;
	T_MRB_ECNL_EPROPERTY *prop;
	int size;

	mrb_get_args(mrb, "oi", &rprp, &rsiz);

	if (!mrb_obj_is_kind_of(mrb, rprp, _class_property)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "data_prop_get");
		return mrb_fixnum_value(0);
	}

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprp);
	size = mrb_fixnum(rsiz);

	rdat = mrb_iv_get(mrb, self, prop->exinf);

	return rdat;
}

static mrb_value mrb_ecnl_enode_initialize(mrb_state *mrb, mrb_value self)
{
	ecn_node_t *nod;
	mrb_int eojx3;
	mrb_value props;
	const mrb_value *rprop;
	int i, count;
	const EPRPINIB **eprp;

	mrb_get_args(mrb, "iA", &eojx3, &props);

	rprop = RARRAY_PTR(props);
	count = RARRAY_LEN(props);

	nod = (ecn_node_t *)mrb_malloc(mrb, sizeof(ecn_node_t) + count * sizeof(EPRPINIB **));
	DATA_TYPE(self) = &mrb_ecnl_enode_type;
	DATA_PTR(self) = nod;

	eprp = (const EPRPINIB **)&nod[1];

	for (i = 0; i < count; i++) {
		T_MRB_ECNL_EPROPERTY *prop;

		if (!mrb_obj_is_kind_of(mrb, rprop[i], _class_property)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "eprpinib_table");
			goto error;
		}

		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		prop->eobj = self;
		eprp[i] = &prop->inib;
	}

	nod->base.inib.eobjatr = EOBJ_LOCAL_NODE;
	nod->base.inib.enodid = 0;
	nod->base.inib.exinf = (intptr_t)NULL;
	nod->base.inib.eojx1 = EOJ_X1_PROFILE;
	nod->base.inib.eojx2 = EOJ_X2_NODE_PROFILE;
	nod->base.inib.eojx3 = eojx3;
	nod->base.inib.eprp = eprp;
	nod->base.inib.eprpcnt = count;
	nod->base.eprpcnt = count;

	return self;
error:
	self = mrb_nil_value();
	mrb_free(mrb, nod);

	return self;
}

static void mrb_ecnl_enode_free(mrb_state *mrb, void *ptr)
{
	ecn_node_t *nod = (ecn_node_t *)ptr;

	/* ノードの設定として取り込まれた場合は破棄しない */
	if(nod->base.svc == NULL)
		mrb_free(mrb, nod);
}

static int mrb_ecnl_prop_set(ecnl_svc_task_t *svc, const EPRPINIB *item, const void *src, int size, bool_t *anno);
static int mrb_ecnl_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);

static mrb_value mrb_ecnl_eproperty_initialize(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;
	mrb_value exinf;
	mrb_value eprpset;
	mrb_value eprpget;
	mrb_int eprpcd;
	mrb_int eprpatr;
	mrb_int eprpsz;
	mrb_sym ei, set, get;

	mrb_get_args(mrb, "iiiooo", &eprpcd, &eprpatr, &eprpsz, &exinf, &eprpset, &eprpget);

	if (mrb_type(exinf) == MRB_TT_SYMBOL) {
		ei = mrb_symbol(exinf);
	}
	else if (mrb_type(exinf) == MRB_TT_FALSE) {
		ei = 0;
	}
	else {
		mrb_raise(mrb, E_RUNTIME_ERROR, "EProperty.new");
		return mrb_nil_value();
	}

	if (mrb_type(eprpset) == MRB_TT_SYMBOL) {
		set = mrb_symbol(eprpset);
	}
	else if (mrb_type(eprpset) == MRB_TT_FALSE) {
		set = 0;
	}
	else {
		mrb_raise(mrb, E_RUNTIME_ERROR, "EProperty.new");
		return mrb_nil_value();
	}

	if (mrb_type(eprpget) == MRB_TT_SYMBOL) {
		get = mrb_symbol(eprpget);
	}
	else if (mrb_type(eprpget) == MRB_TT_FALSE) {
		get = 0;
	}
	else {
		mrb_raise(mrb, E_RUNTIME_ERROR, "EProperty.new");
		return mrb_nil_value();
	}

	prop = (T_MRB_ECNL_EPROPERTY *)mrb_malloc(mrb, sizeof(T_MRB_ECNL_EPROPERTY));
	DATA_TYPE(self) = &mrb_ecnl_eproperty_type;
	DATA_PTR(self) = prop;

	prop->inib.eprpcd = eprpcd;
	prop->inib.eprpatr = eprpatr;
	prop->inib.eprpsz = eprpsz;
	prop->inib.exinf = 0;
	prop->inib.eprpset = mrb_ecnl_prop_set;
	prop->inib.eprpget = mrb_ecnl_prop_get;
	prop->exinf = ei;
	prop->eprpset = set;
	prop->eprpget = get;
	prop->anno = false;

	return self;
}

static void mrb_ecnl_eproperty_free(mrb_state *mrb, void *ptr)
{
	T_MRB_ECNL_EPROPERTY *prop = (T_MRB_ECNL_EPROPERTY *)ptr;
	ecn_obj_t *eobj;

	if (ptr == NULL)
		return;

	if (mrb_type(prop->eobj) != MRB_TT_DATA)
		return;

	eobj= (ecn_obj_t *)DATA_PTR(prop->eobj);

	/* プロパティの設定として取り込まれた場合は破棄しない */
	if (eobj->svc != NULL)
		return;

	mrb_free(mrb, prop);
}

/* ECHONET Lite プロパティコード */
static mrb_value mrb_ecnl_eproperty_get_pcd(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	return mrb_fixnum_value(prop->inib.eprpcd);
}

/* ECHONET Lite プロパティ属性 */
static mrb_value mrb_ecnl_eproperty_get_atr(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	return mrb_fixnum_value(prop->inib.eprpatr);
}

/* ECHONET Lite プロパティのサイズ */
static mrb_value mrb_ecnl_eproperty_get_sz(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	return mrb_fixnum_value(prop->inib.eprpsz);
}

/* ECHONET Lite プロパティの拡張情報 */
static mrb_value mrb_ecnl_eproperty_get_exinf(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	if (prop->inib.exinf == 0)
		return mrb_nil_value();

	return mrb_symbol_value((mrb_sym)prop->inib.exinf);
}

/* ECHONET Lite プロパティの設定関数 */
static mrb_value mrb_ecnl_eproperty_get_setcb(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	if (prop->eprpset == 0)
		return mrb_nil_value();

	return mrb_symbol_value(prop->eprpset);
}

/* ECHONET Lite プロパティの取得関数 */
static mrb_value mrb_ecnl_eproperty_get_getcb(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	if (prop->eprpget == 0)
		return mrb_nil_value();

	return mrb_symbol_value(prop->eprpget);
}

static T_MRB_ECNL_EPROPERTY *cast_prop(const EPRPINIB *inib)
{
	return (T_MRB_ECNL_EPROPERTY *)((intptr_t)inib - offsetof(T_MRB_ECNL_EPROPERTY, inib));
}

/*
 * データ設定関数
 */
static int mrb_ecnl_prop_set(ecnl_svc_task_t *svc, const EPRPINIB *item, const void *src, int size, bool_t *anno)
{
	mrb_state *mrb = svc->mrb;
	T_MRB_ECNL_EPROPERTY *prop = cast_prop(item);
	mrb_value args[2];
	mrb_value ret;

	if (prop->eprpset == 0)
		return 0;

	if (*anno)
		prop->anno = *anno;

	args[0] = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_property));
	DATA_TYPE(args[0]) = &mrb_ecnl_eproperty_type;
	DATA_PTR(args[0]) = prop;

	args[1] = mrb_str_new(mrb, src, size);

	ret = mrb_funcall_argv(mrb, prop->eobj, prop->eprpset, 2, args);

	if (mrb_type(ret) != MRB_TT_FIXNUM) {
		//mrb_raise(mrb, E_RUNTIME_ERROR, "eprpset");
		return 0;
	}

	*anno = prop->anno;

	return mrb_fixnum(ret);
}

/*
* データ取得関数
*/
static int mrb_ecnl_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	mrb_state *mrb = svc->mrb;
	T_MRB_ECNL_EPROPERTY *prop = cast_prop(item);
	mrb_value args[2];
	mrb_value ret;
	int len;

	if (prop->eprpget == 0)
		return 0;

	args[0] = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_property));
	DATA_TYPE(args[0]) = &mrb_ecnl_eproperty_type;
	DATA_PTR(args[0]) = prop;

	args[1] = mrb_fixnum_value(size);

	ret = mrb_funcall_argv(mrb, prop->eobj, prop->eprpget, 2, args);

	if (mrb_type(ret) != MRB_TT_STRING) {
		//mrb_raise(mrb, E_RUNTIME_ERROR, "eprpget");
		return 0;
	}

	len = RSTRING_LEN(ret);
	if (size > len)
		size = len;

	memcpy(dst, RSTRING_PTR(ret), size);

	return size;
}

static mrb_value mrb_ecnl_edata_initialize(mrb_state *mrb, mrb_value self)
{
	T_EDATA *dat;

	dat = (T_EDATA *)mrb_malloc(mrb, sizeof(T_EDATA));
	dat->trn_pos = -1;
	DATA_TYPE(self) = &mrb_ecnl_edata_type;
	DATA_PTR(self) = dat;

	return self;
}

static void mrb_ecnl_edata_free(mrb_state *mrb, void *ptr)
{
	T_EDATA *dat = (T_EDATA *)ptr;
	mrb_free(mrb, dat);
}

mrb_value mrb_ecnl_edata_new(mrb_state *mrb, T_EDATA *data)
{
	mrb_value resv;

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = data;

	return resv;
}
/* プロパティ値書き込み・読み出し要求電文折り返し指定 */
static mrb_value mrb_ecnl_trn_set_get(mrb_state *mrb, mrb_value self)
{
	T_EDATA *edat;
	ER ret;

	edat = (T_EDATA *)DATA_PTR(self);
	ret = ecn_trn_set_get(mrb, edat, &edat->trn_pos);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "trn_set_get");
		return self;
	}

	return self;
}

/* 要求電文へのプロパティ指定追加 */
static mrb_value mrb_ecnl_add_epc(mrb_state *mrb, mrb_value self)
{
	mrb_int epc;
	T_EDATA *edat;
	ER ret;

	mrb_get_args(mrb, "i", &epc);

	edat = (T_EDATA *)DATA_PTR(self);
	ret = ecn_add_epc(mrb, edat, epc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "add_epc");
		return self;
	}

	return self;
}

/* 要求電文へのプロパティデータ追加 */
static mrb_value mrb_ecnl_add_edt(mrb_state *mrb, mrb_value self)
{
	mrb_int epc;
	mrb_value redt;
	T_EDATA *edat;
	ER ret;

	mrb_get_args(mrb, "iS", &epc, &redt);

	edat = (T_EDATA *)DATA_PTR(self);
	ret = ecn_add_edt(mrb, edat, epc, RSTRING_LEN(redt), RSTRING_PTR(redt));
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "add_edt");
		return self;
	}

	return self;
}

static mrb_value mrb_ecnl_get_esv(mrb_state *mrb, mrb_value self)
{
	mrb_value result;
	T_EDATA *edat;

	edat = (T_EDATA *)DATA_PTR(self);
	result = mrb_fixnum_value(edat->hdr.edata.esv);

	return result;
}

static mrb_value mrb_ecnl_eiterator_initialize(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EITERATOR *itr;

	itr = (T_MRB_ECNL_EITERATOR *)mrb_malloc(mrb, sizeof(T_MRB_ECNL_EITERATOR));
	DATA_TYPE(self) = &mrb_ecnl_eiterator_type;
	DATA_PTR(self) = itr;

	return self;
}

static void mrb_ecnl_eiterator_free(mrb_state *mrb, void *ptr)
{
	T_MRB_ECNL_EITERATOR *itr = (T_MRB_ECNL_EITERATOR *)ptr;
	mrb_free(mrb, itr);
}


/* 応答電文解析イテレーター初期化 */
static mrb_value mrb_ecnl_itr_ini(mrb_state *mrb, mrb_value self)
{
	mrb_value ritr;
	T_EDATA *edat;
	T_MRB_ECNL_EITERATOR *eitr;
	ER ret;

	ritr = mrb_obj_new(mrb, _class_iterator, 0, NULL);
	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(ritr);
	eitr->state = 0;
	eitr->is_eof = false;

	edat = (T_EDATA *)DATA_PTR(self);
	ret = ecn_itr_ini(&eitr->itr, edat);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "itr_ini");
		return self;
	}

	return ritr;
}

/* 応答電文解析イテレーターインクリメント */
static mrb_value mrb_ecnl_itr_nxt(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EITERATOR *eitr;
	ER ret;

	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(self);
	ret = ecn_itr_nxt(mrb, &eitr->itr, &eitr->epc, &eitr->pdc, &eitr->edt);
	if (ret == E_BOVR) {
		eitr->state++;
		eitr->is_eof = eitr->itr.is_eof;
	}
	else if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "itr_nxt");
		return self;
	}

	return self;
}

static mrb_value mrb_ecnl_iterator_get_epc(mrb_state *mrb, mrb_value self)
{
	mrb_value result;
	T_MRB_ECNL_EITERATOR *eitr;

	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(self);
	result = mrb_fixnum_value(eitr->epc);

	return result;
}

static mrb_value mrb_ecnl_iterator_get_edt(mrb_state *mrb, mrb_value self)
{
	mrb_value result;
	T_MRB_ECNL_EITERATOR *eitr;

	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(self);
	result = mrb_str_new(mrb, (char *)eitr->edt, eitr->pdc);

	return result;
}

static mrb_value mrb_ecnl_iterator_get_state(mrb_state *mrb, mrb_value self)
{
	mrb_value result;
	T_MRB_ECNL_EITERATOR *eitr;

	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(self);
	result = mrb_fixnum_value(eitr->state);

	return result;
}

static mrb_value mrb_ecnl_iterator_is_eof(mrb_state *mrb, mrb_value self)
{
	mrb_value result;
	T_MRB_ECNL_EITERATOR *eitr;

	eitr = (T_MRB_ECNL_EITERATOR *)DATA_PTR(self);
	result = eitr->is_eof ? mrb_true_value() : mrb_false_value();

	return result;
}

static mrb_value mrb_ecnl_svctask_initialize(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;
	mrb_value profile;
	ecn_node_t *node;
	mrb_value devices;
	const mrb_value *eobjs;
	ecn_device_t *device;
	EOBJCB *eobjcb;
	int i, count;
	ID id = 1;
	ER ret;

	profile = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@profile"));
	if (mrb_type(profile) == MRB_TT_FALSE) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "@profile");
		return mrb_nil_value();
	}

	if (!mrb_obj_is_kind_of(mrb, profile, _class_node)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "@profile");
		return mrb_nil_value();
	}

	devices = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@devices"));
	if (mrb_type(devices) == MRB_TT_FALSE) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "@devices");
		return mrb_nil_value();
	}

	if (mrb_type(devices) != MRB_TT_ARRAY) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "@devices");
		return mrb_nil_value();
	}

	node = (ecn_node_t *)DATA_PTR(profile);
	eobjs = RARRAY_PTR(devices);
	count = RARRAY_LEN(devices);

	svc = (ecnl_svc_task_t *)mrb_malloc(mrb, sizeof(ecnl_svc_task_t) + (1 + count) * sizeof(EOBJINIB *));
	DATA_TYPE(self) = &mrb_ecnl_svctask_type;
	DATA_PTR(self) = svc;

	memset(&svc->agent, 0, sizeof(svc->agent));
	memset(&svc->enodadrb_table, 0, sizeof(svc->enodadrb_table));
	memset(&svc->api_mbxid, 0, sizeof(svc->api_mbxid));
	memset(&svc->svc_mbxid, 0, sizeof(svc->svc_mbxid));
	memset(&svc->lcl_mbxid, 0, sizeof(svc->lcl_mbxid));
	svc->eobjlist_need_init = 1;
	svc->current_tid = 1;

	svc->mrb = mrb;
	svc->self = self;
	svc->tnum_enodid = 1; /* この版ではローカルノード１つ */
	svc->tmax_eobjid = 1 + 1 + count;
	svc->eobjinib_table = (const EOBJINIB **)&svc[1];
	svc->eobjinib_table[0] = &node->base.inib;
	svc->tnum_enodadr = TNUM_ENODADR;

	eobjcb = &svc->eobjcb_table[0];
	eobjcb->eobjs = &svc->eobjinib_table[1];

	node->base.svc = svc;
	node->base.eobjId = id++;
	node->base.inib.enodid = 0;
	eobjcb->profile = &node->base.inib;
	eobjcb->eobjcnt = count;

	for (i = 0; i < count; i++) {
		if (!mrb_obj_is_kind_of(mrb, eobjs[i], _class_object)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "devices");
			goto error;
		}
		device = (ecn_device_t *)DATA_PTR(eobjs[i]);
		device->base.svc = svc;
		device->base.eobjId = id++;
		device->base.inib.enodid = node->base.eobjId;
		eobjcb->eobjs[i] = &device->base.inib;
	}

	/* ECHONETミドルウェアを起動 */
	ret = ecn_sta_svc(svc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "SvcTask.new");
		goto error;
	}

	return self;
error:
	self = mrb_nil_value();
	mrb_free(mrb, svc);

	return self;
}

static ecn_obj_t *cast_obj2(const EOBJINIB *inib)
{
	return (ecn_obj_t *)((intptr_t)inib - offsetof(ecn_obj_t, inib));
}

static void mrb_ecnl_svctask_free(mrb_state *mrb, void *ptr)
{
	ecnl_svc_task_t *svc = (ecnl_svc_task_t *)ptr;
	const EOBJINIB **table = svc->eobjinib_table;
	const EOBJINIB **end = &table[svc->tmax_eobjid];

	for (; table < end; table++) {
		mrb_free(mrb, cast_obj2(*table));
	}

	mrb_free(mrb, svc);
}

const EOBJINIB *echonet_svctask_get_eobjinib(ecnl_svc_task_t *svc, ID eobjid)
{
	return svc->eobjinib_table[eobjid - 1];
}

ID echonet_svctask_get_eobjid(ecnl_svc_task_t *svc, const EOBJINIB *eobjinib)
{
	int i, count = svc->tmax_eobjid;

	for (i = 0; i < count; i++) {
		if (svc->eobjinib_table[i] == eobjinib) {
			return i + 1;
		}
	}

	return 0;
}

/* プロパティ値書き込み要求（応答不要）電文作成 */
static mrb_value mrb_ecnl_esv_set_i(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_int epc;
	mrb_value redt;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ER ret;
	ID eobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oiS", &reobj, &epc, &redt);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	ret = ecn_esv_seti(svc, &edat, eobjid, epc, RSTRING_LEN(redt), RSTRING_PTR(redt));
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_set_i");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* プロパティ値書き込み要求（応答要）電文作成 */
static mrb_value mrb_ecnl_esv_set_c(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_int epc;
	mrb_int pdc;
	mrb_value redt;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ER ret;
	ID eobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oiS", &reobj, &epc, &pdc, &redt);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	ret = ecn_esv_setc(svc, &edat, eobjid, epc, RSTRING_LEN(redt), RSTRING_PTR(redt));
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_set_c");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* プロパティ値読み出し要求電文作成 */
static mrb_value mrb_ecnl_esv_get(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_int epc;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ER ret;
	ID eobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oi", &reobj, &epc);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	ret = ecn_esv_get(svc, &edat, eobjid, epc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_get");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* プロパティ値通知要求電文作成 */
static mrb_value mrb_ecnl_esv_inf_req(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_int epc;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ER ret;
	ID eobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oi", &reobj, &epc);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	ret = ecn_esv_inf_req(svc, &edat, eobjid, epc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_inf_req");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* プロパティ値書き込み・読み出し要求電文作成 */
static mrb_value mrb_ecnl_esv_set_get(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_int epc;
	mrb_value redt;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ER ret;
	ID eobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oiS", &reobj, &epc, &redt);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	ret = ecn_esv_set_get(svc, &edat, eobjid, epc, RSTRING_LEN(redt), RSTRING_PTR(redt));
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_set_get");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* プロパティ値通知（応答要）電文作成 */
static mrb_value mrb_ecnl_esv_infc(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	mrb_value reobj;
	mrb_value rseobj;
	mrb_int sepc;
	T_EDATA *edat;
	ecn_device_t *eobj;
	ecn_device_t *seobj;
	ER ret;
	ID eobjid, seobjid;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "ooi", &reobj, &rseobj, &sepc);

	if (mrb_nil_p(reobj)) {
		eobjid = EOBJ_NULL;
	}
	else {
		eobj = (ecn_device_t *)DATA_PTR(reobj);
		eobjid = eobj->base.eobjId;
	}

	if (mrb_nil_p(rseobj)) {
		seobjid = EOBJ_NULL;
	}
	else {
		seobj = (ecn_device_t *)DATA_PTR(rseobj);
		seobjid = seobj->base.eobjId;
	}

	ret = ecn_esv_infc(svc, &edat, eobjid, seobjid, sepc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "esv_infc");
		return self;
	}

	resv = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_data));
	DATA_TYPE(resv) = &mrb_ecnl_edata_type;
	DATA_PTR(resv) = edat;

	return resv;
}

/* 要求電文の送信 */
static mrb_value mrb_ecnl_snd_esv(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	T_EDATA *edat;
	ER ret;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "o", &resv);

	if (!mrb_obj_is_kind_of(mrb, resv, _class_data)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "snd_esv");
		return mrb_nil_value();
	}

	edat = (T_EDATA *)DATA_PTR(resv);
	DATA_PTR(resv) = NULL;

	if (edat->trn_pos != -1) {
		ret = ecn_end_set_get(mrb, edat, edat->trn_pos);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "end_set_get");
			return self;
		}
	}

	ret = ecn_snd_esv(svc, edat);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "snd_esv");
		return self;
	}

	return self;
}

/* 電文の破棄 */
static mrb_value mrb_ecnl_rel_esv(mrb_state *mrb, mrb_value self)
{
	mrb_value resv;
	T_EDATA *edat;
	ER ret;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "o", &resv);

	if (!mrb_obj_is_kind_of(mrb, resv, _class_data)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "rel_esv");
		return mrb_nil_value();
	}

	edat = (T_EDATA *)DATA_PTR(resv);
	DATA_PTR(resv) = NULL;

	ret = ecn_rel_esv(mrb, edat);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "rel_esv");
		return self;
	}

	return self;
}

static mrb_value mrb_ecnl_svctask_ntf_inl(mrb_state *mrb, mrb_value self)
{
	ER ret;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	/* インスタンスリスト通知の送信 */
	ret = _ecn_tsk_ntf_inl(svc);
	if (ret != E_OK) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "ntf_inl");
		return self;
	}

	return self;
}

static mrb_value mrb_ecnl_svctask_get_timer(mrb_state *mrb, mrb_value self)
{
	TMO timer1, timer2;
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	timer1 = echonet_svctask_get_timer(svc);
	timer2 = echonet_lcltask_get_timer(svc);

	if ((timer1 == TMO_FEVR) || (timer1 > timer2))
		timer1 = timer2;

	return mrb_fixnum_value(timer1);
}

static mrb_value mrb_ecnl_svctask_progress(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;
	TMO timer;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "i", &timer);

	echonet_svctask_progress(svc, timer);
	echonet_lcltask_progress(svc, timer);

	return mrb_nil_value();
}

/*
 * Echonet電文受信処理
 */
static void main_recv_esv(ecnl_svc_task_t *svc, T_EDATA *esv)
{
	mrb_state *mrb = svc->mrb;
	mrb_value resv;

	resv = mrb_ecnl_edata_new(mrb, esv);

	mrb_funcall(mrb, svc->self, "recv_esv", 1, resv);
}

/*
 * 応答電文待ちの割り込み処理
 */
static void main_break_wait(ecnl_svc_task_t *svc, uint8_t *brkdat, int32_t len)
{
	mrb_state *mrb = svc->mrb;
	mrb_value str;

	str = mrb_str_new(mrb, (char *)brkdat, len);

	mrb_funcall(mrb, svc->self, "break_wait", 1, str);
}

void echonet_apptask_recv_msg(ecnl_svc_task_t *svc, T_ECN_FST_BLK *p_msg)
{
	mrb_state *mrb = svc->mrb;
	uint8_t brkdat[64];
	int len;
	ER ret;

	/* Echonet電文受信の場合 */
	if ((p_msg)->hdr.type == ECN_MSG_ECHONET) {
		/* Echonet電文受信処理 */
		main_recv_esv(svc, (T_EDATA *)p_msg);

		/* 領域解放 */
		ret = ecn_rel_esv(NULL, (T_EDATA *)p_msg);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "ecn_rel_esv");
			return;
		}
	}
	/* 応答電文待ちの割り込みの場合 */
	else if ((p_msg)->hdr.type == ECN_MSG_INTERNAL) {
		/* 応答電文待ちの割り込みデータ取得 */
		ret = ecn_get_brk_dat(NULL, (T_EDATA *)p_msg, brkdat, sizeof(brkdat), &len);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "ecn_get_brk_dat");
			return;
		}

		/* 応答電文待ちの割り込み処理 */
		main_break_wait(svc, brkdat, len);

		/* 領域解放 */
		ret = ecn_rel_esv(NULL, (T_EDATA *)p_msg);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "ecn_rel_esv");
			return;
		}
	}
}

static void mrb_ecnl_svctask_proccess(ecnl_svc_task_t *svc)
{
	T_ECN_FST_BLK *p_msg = NULL;
	ER ret;
	int i;

	do {
		i = 0;

		ret = ecn_fbs_dequeue(&svc->svc_mbxid, &p_msg);
		if (ret == E_OK) {
			echonet_svctask_recv_msg(svc, p_msg);
			i++;
		}

		ret = ecn_fbs_dequeue(&svc->lcl_mbxid, &p_msg);
		if (ret == E_OK) {
			echonet_lcltask_recv_msg(svc, p_msg);
			i++;
		}

		ret = ecn_fbs_dequeue(&svc->api_mbxid, &p_msg);
		if (ret == E_OK) {
			echonet_apptask_recv_msg(svc, p_msg);
			i++;
		}
	} while (i != 0);
}

/* 通信レイヤーからの入力 */
static mrb_value mrb_ecnl_svctask_recv_msg(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;
	mrb_value ep;
	mrb_value data;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "oS", &ep, &data);

	echonet_lcl_input_msg(svc, ep, data);

	mrb_ecnl_svctask_proccess(svc);

	return mrb_nil_value();
}

static mrb_value mrb_ecnl_svctask_timeout(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	echonet_svctask_timeout(svc);
	echonet_lcltask_timeout(svc);

	mrb_ecnl_svctask_proccess(svc);

	return mrb_nil_value();
}

static mrb_value mrb_ecnl_svctask_is_match(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;
	mrb_value enod;
	EOBJCB *enodcb;
	mrb_value edata;
	T_EDATA *edat;
	mrb_value ep;
	bool_t ret;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "ooo", &enod, &edata, &ep);

	enodcb = (EOBJCB *)DATA_PTR(enod);
	edat = (T_EDATA *)DATA_PTR(edata);

	ret = ecn_is_match(svc, enodcb, edat, ep);

	return mrb_bool_value(ret);
}

bool_t lcl_is_local_addr(ecnl_svc_task_t *svc, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "is_local_addr", 1, ep);

	if (mrb_type(ret) == MRB_TT_TRUE)
		return true;

	if (mrb_type(ret) == MRB_TT_FALSE)
		return false;

	mrb_raise(mrb, E_RUNTIME_ERROR, "is_local_addr");
	return false;
}

bool_t lcl_is_multicast_addr(ecnl_svc_task_t *svc, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "is_multicast_addr", 1, ep);

	if (mrb_type(ret) == MRB_TT_TRUE)
		return true;

	if (mrb_type(ret) == MRB_TT_FALSE)
		return false;

	mrb_raise(mrb, E_RUNTIME_ERROR, "is_multicast_addr");
	return false;
}

bool_t lcl_equals_addr(ecnl_svc_task_t *svc, mrb_value ep1, mrb_value ep2)
{
	mrb_state *mrb = svc->mrb;
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "equals_addr", 2, ep1, ep2);

	if (mrb_type(ret) == MRB_TT_TRUE)
		return true;

	if (mrb_type(ret) == MRB_TT_FALSE)
		return false;

	mrb_raise(mrb, E_RUNTIME_ERROR, "equals_addr");
	return false;
}

mrb_value lcl_get_local_addr(ecnl_svc_task_t *svc)
{
	return mrb_funcall(svc->mrb, svc->self, "get_local_addr", 0);
}

mrb_value lcl_get_multicast_addr(ecnl_svc_task_t *svc)
{
	return mrb_funcall(svc->mrb, svc->self, "get_multicast_addr", 0);
}

bool_t lcl_is_match(ecnl_svc_task_t *svc, struct ecn_node *enodcb, T_EDATA *edata, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value enod = mrb_obj_value(enodcb);
	mrb_value edat = mrb_obj_value(edata);
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "is_match", 3, enod, edat, ep);

	if (mrb_type(ret) == MRB_TT_TRUE)
		return true;

	if (mrb_type(ret) == MRB_TT_FALSE)
		return false;

	mrb_raise(mrb, E_RUNTIME_ERROR, "is_match");
	return false;
}

ER lcl_snd_msg(ecnl_svc_task_t *svc, mrb_value ep, mrb_value msg)
{
	mrb_funcall(svc->mrb, svc->self, "snd_msg", 2, ep, msg);

	return E_OK;
}

void mrb_mruby_ecnl_gem_init(mrb_state *mrb)
{
	_module_ecnl = mrb_define_module(mrb, "ECNL");

	/* 未設定 */
	mrb_define_const(mrb, _module_ecnl, "EPC_NONE", mrb_fixnum_value(EPC_NONE));
	/* アクセスルール Set */
	mrb_define_const(mrb, _module_ecnl, "EPC_RULE_SET", mrb_fixnum_value(EPC_RULE_SET));
	/* アクセスルール Get */
	mrb_define_const(mrb, _module_ecnl, "EPC_RULE_GET", mrb_fixnum_value(EPC_RULE_GET));
	/* アクセスルール Anno */
	mrb_define_const(mrb, _module_ecnl, "EPC_RULE_ANNO", mrb_fixnum_value(EPC_RULE_ANNO));
	/* 状態変化時通知 */
	mrb_define_const(mrb, _module_ecnl, "EPC_ANNOUNCE", mrb_fixnum_value(EPC_ANNOUNCE));
	/* 可変長データ */
	mrb_define_const(mrb, _module_ecnl, "EPC_VARIABLE", mrb_fixnum_value(EPC_VARIABLE));

	/* プロパティ値書き込み要求（応答不要）		*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_I", mrb_fixnum_value(ESV_SET_I));
	/* プロパティ値書き込み要求（応答要）		*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_C", mrb_fixnum_value(ESV_SET_C));
	/* プロパティ値読み出し要求					*/
	mrb_define_const(mrb, _module_ecnl, "ESV_GET", mrb_fixnum_value(ESV_GET));
	/* プロパティ値通知要求						*/
	mrb_define_const(mrb, _module_ecnl, "ESV_INF_REQ", mrb_fixnum_value(ESV_INF_REQ));
	/* プロパティ値書き込み・読み出し要求		*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_GET", mrb_fixnum_value(ESV_SET_GET));
	/* プロパティ値書き込み応答					*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_RES", mrb_fixnum_value(ESV_SET_RES));
	/* プロパティ値読み出し応答					*/
	mrb_define_const(mrb, _module_ecnl, "ESV_GET_RES", mrb_fixnum_value(ESV_GET_RES));
	/* プロパティ値通知							*/
	mrb_define_const(mrb, _module_ecnl, "ESV_INF", mrb_fixnum_value(ESV_INF));
	/* プロパティ値通知（応答要）				*/
	mrb_define_const(mrb, _module_ecnl, "ESV_INFC", mrb_fixnum_value(ESV_INFC));
	/* プロパティ値通知応答						*/
	mrb_define_const(mrb, _module_ecnl, "ESV_INFC_RES", mrb_fixnum_value(ESV_INFC_RES));
	/* プロパティ値書き込み・読み出し応答		*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_GET_RES", mrb_fixnum_value(ESV_SET_GET_RES));
	/* プロパティ値書き込み要求不可応答			*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_I_SNA", mrb_fixnum_value(ESV_SET_I_SNA));
	/* プロパティ値書き込み要求不可応答			*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_C_SNA", mrb_fixnum_value(ESV_SET_C_SNA));
	/* プロパティ値読み出し不可応答				*/
	mrb_define_const(mrb, _module_ecnl, "ESV_GET_SNA", mrb_fixnum_value(ESV_GET_SNA));
	/* プロパティ値通知不可応答					*/
	mrb_define_const(mrb, _module_ecnl, "ESV_INF_SNA", mrb_fixnum_value(ESV_INF_SNA));
	/* プロパティ値書き込み・読み出し不可応答	*/
	mrb_define_const(mrb, _module_ecnl, "ESV_SET_GET_SNA", mrb_fixnum_value(ESV_SET_GET_SNA));

	_class_object = mrb_define_class_under(mrb, _module_ecnl, "EObject", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_object, MRB_TT_DATA);
	mrb_define_method(mrb, _class_object, "initialize", mrb_ecnl_eobject_initialize, MRB_ARGS_REQ(5));

	/* プロパティ値書き込み */
	mrb_define_method(mrb, _class_object, "data_prop_set", mrb_ecnl_eobject_data_prop_set, MRB_ARGS_REQ(4));

	/* プロパティ値読み出し */
	mrb_define_method(mrb, _class_object, "data_prop_get", mrb_ecnl_eobject_data_prop_get, MRB_ARGS_REQ(3));

	_class_node = mrb_define_class_under(mrb, _module_ecnl, "ENode", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_node, MRB_TT_DATA);
	mrb_define_method(mrb, _class_node, "initialize", mrb_ecnl_enode_initialize, MRB_ARGS_REQ(2));

	/* プロパティ値書き込み */
	mrb_define_method(mrb, _class_node, "data_prop_set", mrb_ecnl_eobject_data_prop_set, MRB_ARGS_REQ(4));

	/* プロパティ値読み出し */
	mrb_define_method(mrb, _class_node, "data_prop_get", mrb_ecnl_eobject_data_prop_get, MRB_ARGS_REQ(3));

	_class_property = mrb_define_class_under(mrb, _module_ecnl, "EProperty", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_property, MRB_TT_DATA);
	mrb_define_method(mrb, _class_property, "initialize", mrb_ecnl_eproperty_initialize, MRB_ARGS_REQ(6));

	/* ECHONET Lite プロパティコード */
	mrb_define_method(mrb, _class_property, "get_pcd", mrb_ecnl_eproperty_get_pcd, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティ属性 */
	mrb_define_method(mrb, _class_property, "get_atr", mrb_ecnl_eproperty_get_atr, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティのサイズ */
	mrb_define_method(mrb, _class_property, "get_sz", mrb_ecnl_eproperty_get_sz, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの拡張情報 */
	mrb_define_method(mrb, _class_property, "get_exinf", mrb_ecnl_eproperty_get_exinf, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの設定関数 */
	mrb_define_method(mrb, _class_property, "get_setcb", mrb_ecnl_eproperty_get_setcb, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの取得関数 */
	mrb_define_method(mrb, _class_property, "get_getcb", mrb_ecnl_eproperty_get_getcb, MRB_ARGS_NONE());

	_class_data = mrb_define_class_under(mrb, _module_ecnl, "EData", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_data, MRB_TT_DATA);
	mrb_define_method(mrb, _class_data, "initialize", mrb_ecnl_edata_initialize, MRB_ARGS_NONE());

	/* プロパティ値書き込み・読み出し要求電文折り返し指定 */
	mrb_define_method(mrb, _class_data, "trn_set_get", mrb_ecnl_trn_set_get, MRB_ARGS_NONE());

	/* 要求電文へのプロパティ指定追加 */
	mrb_define_method(mrb, _class_data, "add_epc", mrb_ecnl_add_epc, MRB_ARGS_REQ(1));

	/* 要求電文へのプロパティデータ追加 */
	mrb_define_method(mrb, _class_data, "add_edt", mrb_ecnl_add_edt, MRB_ARGS_REQ(2));

	/* 応答電文サービスコード取得 */
	mrb_define_method(mrb, _class_data, "get_esv", mrb_ecnl_get_esv, MRB_ARGS_NONE());

	/* 応答電文解析イテレーター初期化 */
	mrb_define_method(mrb, _class_data, "itr_ini", mrb_ecnl_itr_ini, MRB_ARGS_NONE());

	_class_iterator = mrb_define_class_under(mrb, _module_ecnl, "EIterator", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_iterator, MRB_TT_DATA);
	mrb_define_method(mrb, _class_iterator, "initialize", mrb_ecnl_eiterator_initialize, MRB_ARGS_NONE());

	/* 応答電文解析イテレーターインクリメント */
	mrb_define_method(mrb, _class_iterator, "itr_nxt", mrb_ecnl_itr_nxt, MRB_ARGS_NONE());

	mrb_define_method(mrb, _class_iterator, "get_epc", mrb_ecnl_iterator_get_epc, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_iterator, "get_edt", mrb_ecnl_iterator_get_edt, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_iterator, "get_state", mrb_ecnl_iterator_get_state, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_iterator, "is_eof", mrb_ecnl_iterator_is_eof, MRB_ARGS_NONE());

	_class_svctask = mrb_define_class_under(mrb, _module_ecnl, "SvcTask", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_svctask, MRB_TT_DATA);
	mrb_define_method(mrb, _class_svctask, "initialize", mrb_ecnl_svctask_initialize, MRB_ARGS_NONE());

	/* プロパティ値書き込み要求（応答不要）電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_set_i", mrb_ecnl_esv_set_i, MRB_ARGS_REQ(3));

	/* プロパティ値書き込み要求（応答要）電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_set_c", mrb_ecnl_esv_set_c, MRB_ARGS_REQ(3));

	/* プロパティ値読み出し要求電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_get", mrb_ecnl_esv_get, MRB_ARGS_REQ(2));

	/* プロパティ値通知要求電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_inf_req", mrb_ecnl_esv_inf_req, MRB_ARGS_REQ(2));

	/* プロパティ値書き込み・読み出し要求電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_set_get", mrb_ecnl_esv_set_get, MRB_ARGS_REQ(3));

	/* プロパティ値通知（応答要）電文作成 */
	mrb_define_method(mrb, _class_svctask, "esv_infc", mrb_ecnl_esv_infc, MRB_ARGS_REQ(3));

	/* 要求電文の送信 */
	mrb_define_method(mrb, _class_svctask, "snd_esv", mrb_ecnl_snd_esv, MRB_ARGS_REQ(1));

	/* 電文の破棄 */
	mrb_define_method(mrb, _class_svctask, "rel_esv", mrb_ecnl_rel_esv, MRB_ARGS_REQ(1));

	/* インスタンスリスト通知の送信 */
	mrb_define_method(mrb, _class_svctask, "ntf_inl", mrb_ecnl_svctask_ntf_inl, MRB_ARGS_NONE());

	/* メッセージ処理ループ */
	mrb_define_method(mrb, _class_svctask, "get_timer", mrb_ecnl_svctask_get_timer, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_svctask, "progress", mrb_ecnl_svctask_progress, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, _class_svctask, "recv_msg", mrb_ecnl_svctask_recv_msg, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, _class_svctask, "timeout", mrb_ecnl_svctask_timeout, MRB_ARGS_NONE());

	/* リモートECHONETノードの適合確認 */
	mrb_define_method(mrb, _class_svctask, "is_match", mrb_ecnl_svctask_is_match, MRB_ARGS_REQ(3));
}

void mrb_mruby_ecnl_gem_final(mrb_state *mrb)
{
}
