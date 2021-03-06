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

const static struct mrb_data_type mrb_ecnl_eobject_type = { "EObject", mrb_ecnl_eobject_free };
const static struct mrb_data_type mrb_ecnl_enode_type = { "ENode", mrb_ecnl_enode_free };
const static struct mrb_data_type mrb_ecnl_eproperty_type = { "EProperty", mrb_ecnl_eproperty_free };
const static struct mrb_data_type mrb_ecnl_edata_type = { "EData", mrb_ecnl_edata_free };
const static struct mrb_data_type mrb_ecnl_eiterator_type = { "EIterator", mrb_ecnl_eiterator_free };
const static struct mrb_data_type mrb_ecnl_svctask_type = { "SvcTask", mrb_ecnl_svctask_free };

static int anno_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int set_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int get_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int inst_count_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int class_count_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int inst_list_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int inst_lists_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);
static int class_lists_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size);

const static EPRPINIB auto_eproperties[] = {
	{0x9D, EPC_RULE_GET, 255, 0, NULL, anno_prpmap_prop_get},
	{0x9E, EPC_RULE_GET, 255, 0, NULL, set_prpmap_prop_get},
	{0x9F, EPC_RULE_GET, 255, 0, NULL, get_prpmap_prop_get},
	{0xD3, EPC_RULE_GET, 255, 0, NULL, inst_count_prop_get},
	{0xD4, EPC_RULE_GET, 255, 0, NULL, class_count_prop_get},
	{0xD5, EPC_RULE_ANNO, 255, 0, NULL, inst_list_prop_get},
	{0xD6, EPC_RULE_GET, 255, 0, NULL, inst_lists_prop_get},
	{0xD7, EPC_RULE_GET, 255, 0, NULL, class_lists_prop_get},
};

static T_MRB_ECNL_EPROPERTY *cast_prop(const EPRPINIB *inib)
{
	return (T_MRB_ECNL_EPROPERTY *)((intptr_t)inib - offsetof(T_MRB_ECNL_EPROPERTY, inib));
}

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
	EPRPINIB *aprops;
	int i, count, icnt;

	mrb_get_args(mrb, "iiioA", &eojx1, &eojx2, &eojx3, &node, &props);

	if (!mrb_obj_is_kind_of(mrb, node, _class_node)) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "EObject.new");
		return mrb_nil_value();
	}

	rprop = RARRAY_PTR(props);
	icnt = RARRAY_LEN(props);
	count = icnt + 3; /* プロパティマップ分 */

	for (i = 0; i < icnt; i++) {
		T_MRB_ECNL_EPROPERTY *prop;

		if (!mrb_obj_is_kind_of(mrb, rprop[i], _class_property)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "eprpinib_table");
			return mrb_nil_value();
		}

		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		/* プロパティマップの場合は減算 */
		switch (prop->inib.eprpcd) {
		case 0x9D: case 0x9E: case 0x9F:
			count--;
			break;
		}
	}

	obj = (ecn_device_t *)mrb_calloc(mrb, 1, sizeof(ecn_device_t) + count * sizeof(EPRPINIB *)
		+ (sizeof(EPRPINIB) * 3));
	DATA_TYPE(self) = &mrb_ecnl_eobject_type;
	DATA_PTR(self) = obj;

	eprp = (const EPRPINIB **)&obj[1];
	aprops = (EPRPINIB *)&eprp[count];

	obj->base.inib.eobjatr = EOBJ_DEVICE;
	obj->base.inib.enodid = 0;
	obj->base.inib.exinf = (intptr_t)NULL;
	obj->base.inib.eojx1 = eojx1;
	obj->base.inib.eojx2 = eojx2;
	obj->base.inib.eojx3 = eojx3;
	obj->base.inib.eprp = eprp;
	obj->base.inib.eprpcnt = count;
	obj->base.eprpcnt = count;

	obj->node = (ecn_node_t *)DATA_PTR(node);

	count = 0;
	for (i = 0; i < icnt; i++) {
		T_MRB_ECNL_EPROPERTY *prop;
		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		/* プロパティマップの場合は無視 */
		switch (prop->inib.eprpcd) {
		case 0x9D: case 0x9E: case 0x9F:
			continue;
		}

		prop->eobj = self;
		eprp[count++] = &prop->inib;
	}

	for (int j = 0; j < 3; j++) {
		EPRPINIB *prop = &aprops[j];

		memcpy(prop, &auto_eproperties[j], sizeof(*prop));

		prop->exinf = (intptr_t)obj;
		eprp[count++] = prop;
	}

	return self;
}

static void mrb_ecnl_eobject_free(mrb_state *mrb, void *ptr)
{
	ecn_device_t *obj = (ecn_device_t *)ptr;
	T_MRB_ECNL_EPROPERTY *prop;

	/* 機器オブジェクトの設定として取り込まれた場合は破棄しない */
	if (obj->base.svc != NULL)
		return;

	for (int i = 0; i < obj->base.eprpcnt; i++) {
		const EPRPINIB *eprp = obj->base.inib.eprp[i];

		switch (eprp->eprpcd) {
		case 0x9D: case 0x9E: case 0x9F:
			continue;
		}

		prop = cast_prop(eprp);

		mrb_free(mrb, prop);
	}

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
	if (RSTRING_LEN(rdat) != size) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "data_prop_get");
		return mrb_nil_value();
	}

	return rdat;
}

static mrb_value mrb_ecnl_enode_initialize(mrb_state *mrb, mrb_value self)
{
	ecn_node_t *nod;
	mrb_int eojx3;
	mrb_value props;
	const mrb_value *rprop;
	int i, count, icnt;
	const EPRPINIB **eprp;
	EPRPINIB *aprops;

	mrb_get_args(mrb, "iA", &eojx3, &props);

	rprop = RARRAY_PTR(props);
	icnt = RARRAY_LEN(props);
	count = icnt + 8; /* インスタンスリストなどの分 */

	for (i = 0; i < icnt; i++) {
		T_MRB_ECNL_EPROPERTY *prop;

		if (!mrb_obj_is_kind_of(mrb, rprop[i], _class_property)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "eprpinib_table");
			return mrb_nil_value();
		}

		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		/* インスタンスリストなどの場合は減算 */
		switch (prop->inib.eprpcd) {
		case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0x9D: case 0x9E: case 0x9F:
			count--;
			break;
		}
	}

	nod = (ecn_node_t *)mrb_calloc(mrb, 1, sizeof(ecn_node_t) + count * sizeof(EPRPINIB *)
		+ (sizeof(EPRPINIB) * 8)/* インスタンスリストなどの分 */);
	DATA_TYPE(self) = &mrb_ecnl_enode_type;
	DATA_PTR(self) = nod;

	eprp = (const EPRPINIB **)&nod[1];
	aprops = (EPRPINIB *)&eprp[count];

	nod->base.inib.eobjatr = EOBJ_LOCAL_NODE;
	nod->base.inib.enodid = 0;
	nod->base.inib.exinf = (intptr_t)NULL;
	nod->base.inib.eojx1 = EOJ_X1_PROFILE;
	nod->base.inib.eojx2 = EOJ_X2_NODE_PROFILE;
	nod->base.inib.eojx3 = eojx3;
	nod->base.inib.eprp = eprp;
	nod->base.inib.eprpcnt = count;
	nod->base.eprpcnt = count;

	count = 0;
	for (i = 0; i < icnt; i++) {
		T_MRB_ECNL_EPROPERTY *prop;
		prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(rprop[i]);

		/* インスタンスリストなどの場合は無視 */
		switch (prop->inib.eprpcd) {
		case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0x9D: case 0x9E: case 0x9F:
			continue;
		}

		prop->eobj = self;
		eprp[count++] = &prop->inib;
	}

	for (int j = 0; j < 8; j++) {
		EPRPINIB *prop = &aprops[j];

		memcpy(prop, &auto_eproperties[j], sizeof(*prop));

		prop->exinf = (intptr_t)nod;
		eprp[count++] = prop;
	}

	return self;
}

static void mrb_ecnl_enode_free(mrb_state *mrb, void *ptr)
{
	ecn_node_t *nod = (ecn_node_t *)ptr;
	T_MRB_ECNL_EPROPERTY *prop;

	/* ノードの設定として取り込まれた場合は破棄しない */
	if (nod->base.svc != NULL)
		return;

	for (int i = 0; i < nod->base.eprpcnt; i++) {
		const EPRPINIB *eprp = nod->base.inib.eprp[i];

		switch (eprp->eprpcd) {
		case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0x9D: case 0x9E: case 0x9F:
			continue;
		}

		prop = cast_prop(eprp);

		mrb_free(mrb, prop);
	}

	mrb_free(mrb, nod);
}

static int anno_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_obj_t *obj = (ecn_obj_t *)item->exinf;
	uint8_t *dmap = &((uint8_t *)dst)[1];
	uint8_t count;

	count = obj->annocnt;
	*(uint8_t *)dst = count;
	if (count < 16) {
		memcpy(dmap, obj->pmapAnno, count);
		return count + 1;
	}
	else {
		memcpy(dmap, obj->pmapAnno, 16);
		return 17;
	}
}

static int set_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_obj_t *obj = (ecn_obj_t *)item->exinf;
	uint8_t *dmap = &((uint8_t *)dst)[1];
	uint8_t count;

	count = obj->setcnt;
	*(uint8_t *)dst = count;
	if (count < 16) {
		memcpy(dmap, obj->pmapSet, count);
		return count + 1;
	}
	else {
		memcpy(dmap, obj->pmapSet, 16);
		return 17;
	}
}

static int get_prpmap_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_obj_t *obj = (ecn_obj_t *)item->exinf;
	uint8_t *dmap = &((uint8_t *)dst)[1];
	uint8_t count;

	count = obj->getcnt;
	*(uint8_t *)dst = count;
	if (count < 16) {
		memcpy(dmap, obj->pmapGet, count);
		return count + 1;
	}
	else {
		memcpy(dmap, obj->pmapGet, 16);
		return 17;
	}
}

static int inst_count_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_node_t *nod = (ecn_node_t *)item->exinf;
	uint8_t *pos = dst;
	EOBJCB *obj = &svc->eobjcb_table[nod->base.eobjId - 1];
	int count = obj->eobjcnt;

	pos[0] = count >> 16;
	pos[1] = count >> 8;
	pos[2] = count;

	return 3;
}

static int class_count_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_node_t *nod = (ecn_node_t *)item->exinf;
	uint8_t *pos = dst;
	int count = svc->eclscnt;

	pos[0] = count >> 8;
	pos[1] = count;

	return 2;
}

static int inst_list_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_node_t *nod = (ecn_node_t *)item->exinf;
	uint8_t *pos = dst;
	EOBJCB *obj = &svc->eobjcb_table[nod->base.eobjId - 1];
	const EOBJINIB **eobjs = obj->eobjs;
	int eobjcnt = obj->eobjcnt;
	int inst_list_pos = svc->inst_list_pos;
	int count = 0;

	/* 通知数の位置を空けておく */
	pos++;
	for (int i = inst_list_pos; (i < eobjcnt) && (count < 84); i++) {
		const EOBJINIB *eobj = eobjs[i];

		*pos++ = eobj->eojx1;
		*pos++ = eobj->eojx2;
		*pos++ = eobj->eojx3;
		count++;
	}
	*(uint8_t *)dst = count;

	inst_list_pos += count;
	/* 最後まで送信し終わっていたら初めから */
	if (inst_list_pos >= eobjcnt)
		inst_list_pos = 0;

	svc->inst_list_pos = inst_list_pos;

	return (int)pos - (int)dst;
}

static int inst_lists_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	ecn_node_t *nod = (ecn_node_t *)item->exinf;
	uint8_t *pos = dst;
	EOBJCB *obj = &svc->eobjcb_table[nod->base.eobjId - 1];
	const EOBJINIB **eobjs = obj->eobjs;
	int eobjcnt = obj->eobjcnt;

	if (eobjcnt < 255)
		*pos++ = eobjcnt;
	else
		*pos++ = 255; /*オーバーフロー*/

	for (int i = 0; (i < eobjcnt) && (i < 84); i++) {
		const EOBJINIB *eobj = eobjs[i];

		*pos++ = eobj->eojx1;
		*pos++ = eobj->eojx2;
		*pos++ = eobj->eojx3;
	}

	return (int)pos - (int)dst;
}

static int class_lists_prop_get(ecnl_svc_task_t *svc, const EPRPINIB *item, void *dst, int size)
{
	uint8_t *pos = dst;
	int eclscnt = svc->eclscnt, len;

	if (eclscnt < 255)
		*pos++ = eclscnt;
	else
		*pos++ = 255; /*オーバーフロー*/

	len = 2 * eclscnt;
	memcpy(pos, svc->eclslst, len);
	pos += len;

	return (int)pos - (int)dst;
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

	if (mrb_type(prop->eobj) == MRB_TT_DATA) {
		eobj = (ecn_obj_t *)DATA_PTR(prop->eobj);

		/* プロパティの設定として取り込まれた場合は破棄しない */
		if (eobj != NULL)
			return;
	}

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

	return mrb_iv_get(mrb, prop->eobj, prop->exinf);
}

/* ECHONET Lite プロパティの拡張情報設定 */
static mrb_value mrb_ecnl_eproperty_set_exinf(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;
	mrb_value exinf;

	mrb_get_args(mrb, "o", &exinf);

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	mrb_iv_set(mrb, prop->eobj, prop->exinf, exinf);

	return exinf;
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

/* ECHONET Lite プロパティの通知有無 */
static mrb_value mrb_ecnl_eproperty_get_anno(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	if (prop->anno)
		return mrb_true_value();

	return mrb_false_value();
}

/* ECHONET Lite プロパティの通知有無設定 */
static mrb_value mrb_ecnl_eproperty_set_anno(mrb_state *mrb, mrb_value self)
{
	T_MRB_ECNL_EPROPERTY *prop;
	mrb_value anno;

	mrb_get_args(mrb, "b", &anno);

	prop = (T_MRB_ECNL_EPROPERTY *)DATA_PTR(self);

	prop->anno = mrb_type(anno) != MRB_TT_FALSE;

	return anno;
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

	prop->anno = *anno;

	args[0] = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, _class_property));
	DATA_TYPE(args[0]) = &mrb_ecnl_eproperty_type;
	DATA_PTR(args[0]) = prop;

	args[1] = mrb_str_new_static(mrb, src, size);

	ret = mrb_funcall_argv(mrb, prop->eobj, prop->eprpset, 2, args);

	if (mrb_type(ret) != MRB_TT_FIXNUM) {
		//mrb_raise(mrb, E_RUNTIME_ERROR, "eprpset");
		return 0;
	}

	if (*anno)
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

void make_prop_map(ecn_obj_t *eobj, uint8_t *prpmap, int count, ATR flag)
{
	if (count < 16) {
		uint8_t *pos = prpmap;
		for (int i = 0; i < eobj->eprpcnt; i++) {
			const EPRPINIB *eprp = eobj->inib.eprp[i];
			if ((eprp->eprpatr & flag) == 0)
				continue;

			*pos++ = eprp->eprpcd;
		}
	}
	else {
		for (int i = 0; i < eobj->eprpcnt; i++) {
			const EPRPINIB *eprp = eobj->inib.eprp[i];
			if ((eprp->eprpatr & flag) == 0)
				continue;

			ecn_agent_set_epc_to_prop_map(eobj->inib.eprp[i]->eprpcd, prpmap);
		}
	}
}

void make_obj_prop_map(ecn_obj_t *eobj)
{
	int annocnt = 0, setcnt = 0, getcnt = 0;

	for (int i = 0; i < eobj->eprpcnt; i++) {
		ATR eprpatr = eobj->inib.eprp[i]->eprpatr;
		if (eprpatr & EPC_RULE_ANNO)
			annocnt++;
		if (eprpatr & EPC_RULE_SET)
			setcnt++;
		if (eprpatr & EPC_RULE_GET)
			getcnt++;
	}
	eobj->annocnt = annocnt;
	eobj->setcnt = setcnt;
	eobj->getcnt = getcnt;

	make_prop_map(eobj, eobj->pmapAnno, annocnt, EPC_RULE_ANNO);
	make_prop_map(eobj, eobj->pmapSet, setcnt, EPC_RULE_SET);
	make_prop_map(eobj, eobj->pmapGet, getcnt, EPC_RULE_GET);
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
	int i, eobjcnt;
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
	eobjcnt = RARRAY_LEN(devices);

	svc = (ecnl_svc_task_t *)mrb_calloc(mrb, 1, sizeof(ecnl_svc_task_t)
		+ (1 + eobjcnt) * sizeof(EOBJINIB *)
		+ (2 * eobjcnt)/*クラスリスト用*/);
	DATA_TYPE(self) = &mrb_ecnl_svctask_type;
	DATA_PTR(self) = svc;

	svc->eobjlist_need_init = 1;
	svc->current_tid = 1;

	svc->mrb = mrb;
	svc->self = self;
	svc->tnum_enodid = 1; /* この版ではローカルノード１つ */
	svc->tmax_eobjid = 1 + 1 + eobjcnt;
	svc->eobjinib_table = (const EOBJINIB **)&svc[1];
	svc->eobjinib_table[0] = &node->base.inib;

	eobjcb = &svc->eobjcb_table[0];
	eobjcb->eobjs = &svc->eobjinib_table[1];

	node->base.svc = svc;
	node->base.eobjId = id++;
	node->base.inib.enodid = 0;
	eobjcb->profile = &node->base.inib;
	eobjcb->eobjcnt = eobjcnt;
	make_obj_prop_map(&node->base);

	for (i = 0; i < eobjcnt; i++) {
		if (!mrb_obj_is_kind_of(mrb, eobjs[i], _class_object)) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "devices");
			goto error;
		}
		device = (ecn_device_t *)DATA_PTR(eobjs[i]);
		device->base.svc = svc;
		device->base.eobjId = id++;
		device->base.inib.enodid = node->base.eobjId;
		eobjcb->eobjs[i] = &device->base.inib;
		make_obj_prop_map(&device->base);
	}

	/* クラスリストの作成 */
	svc->eclslst = (uint8_t *)&svc->eobjinib_table[1 + eobjcnt];
	uint8_t *pos = svc->eclslst;
	int eclscnt = 0;
	for (i = 0; i < eobjcnt; i++) {
		const EOBJINIB *eobj = eobjcb->eobjs[i];

		uint8_t *pos2 = svc->eclslst;
		bool_t match = false;
		for (int j = 0; j < eclscnt; j++, pos2 += 2) {
			const EOBJINIB *eobj2 = eobjcb->eobjs[j];

			match = (pos2[0] == eobj2->eojx1) && (pos2[1] = eobj2->eojx2);
			if (match)
				break;
		}
		if (match)
			continue;

		*pos++ = eobj->eojx1;
		*pos++ = eobj->eojx2;
		eclscnt++;
	}
	svc->eclscnt = eclscnt;

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
#if 0 /* どこかで解放しているらしい･･･ */
	const EOBJINIB **table = svc->eobjinib_table;
	const EOBJINIB **end = &table[svc->tmax_eobjid];

	for (; table < end; table++) {
		mrb_free(mrb, cast_obj2(*table));
	}
#endif
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

static mrb_value mrb_ecnl_svctask_set_timer(mrb_state *mrb, mrb_value self)
{
	TMO timer;
	ecnl_svc_task_t *svc;

	mrb_get_args(mrb, "i", &timer);

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->api_timer = timer;

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

	timer2 = svc->api_timer;
	if ((timer1 == TMO_FEVR) || (timer1 > timer2))
		timer1 = timer2;

	return mrb_fixnum_value(timer1);
}

static mrb_value mrb_ecnl_svctask_progress(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;
	TMO interval;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	mrb_get_args(mrb, "i", &interval);

	echonet_svctask_progress(svc, interval);
	echonet_lcltask_progress(svc, interval);

	svc->api_timer -= interval;
	if (svc->api_timer < 0) {
		svc->api_timer = 0;
	}

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

		/* 領域解放はGCに任せる
		ret = ecn_rel_esv(mrb, (T_EDATA *)p_msg);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "ecn_rel_esv");
			return;
		} */
	}
	/* 応答電文待ちの割り込みの場合 */
	else if ((p_msg)->hdr.type == ECN_MSG_INTERNAL) {
		/* 応答電文待ちの割り込みデータ取得 */
		ret = ecn_get_brk_dat(mrb, (T_EDATA *)p_msg, brkdat, sizeof(brkdat), &len);
		if (ret != E_OK) {
			mrb_raise(mrb, E_RUNTIME_ERROR, "ecn_get_brk_dat");
			return;
		}

		/* 応答電文待ちの割り込み処理 */
		main_break_wait(svc, brkdat, len);

		/* 領域解放 */
		ret = ecn_rel_esv(mrb, (T_EDATA *)p_msg);
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

static mrb_value mrb_ecnl_svctask_call_timeout(mrb_state *mrb, mrb_value self)
{
	ecnl_svc_task_t *svc;

	svc = (ecnl_svc_task_t *)DATA_PTR(self);
	svc->mrb = mrb;

	echonet_svctask_timeout(svc);
	echonet_lcltask_timeout(svc);

	if (svc->api_timer == 0) {
		svc->api_timer = -1;

		mrb_funcall(mrb, svc->self, "timeout", 0);
	}

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

bool_t lcl_is_valid_addrid(ecnl_svc_task_t *svc, ECN_ENOD_ID id)
{
	mrb_state *mrb = svc->mrb;
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "is_valid_addrid", 1, mrb_fixnum_value(id));

	if (mrb_type(ret) == MRB_TT_TRUE)
		return true;

	if (mrb_type(ret) == MRB_TT_FALSE)
		return false;

	mrb_raise(mrb, E_RUNTIME_ERROR, "is_valid_addrid");
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

mrb_value lcl_get_remote_addr(ecnl_svc_task_t *svc, ECN_ENOD_ID id)
{
	return mrb_funcall(svc->mrb, svc->self, "get_remote_addr", 1, mrb_fixnum_value(id));
}

ECN_ENOD_ID lcl_get_remote_id(ecnl_svc_task_t *svc, const mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "get_remote_id", 1, ep);

	if (mrb_type(ret) == MRB_TT_FALSE)
		return ENOD_NOT_MATCH_ID;

	if (mrb_type(ret) == MRB_TT_FIXNUM)
		return (ECN_ENOD_ID)mrb_fixnum(ret);

	return ENOD_NOT_MATCH_ID;
}

ECN_ENOD_ID lcl_set_remote_addr(ecnl_svc_task_t *svc, T_EDATA *edata, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value edat = mrb_obj_value(edata);
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "set_remote_addr", 2, edat, ep);

	if (mrb_type(ret) == MRB_TT_FALSE)
		return ENOD_NOT_MATCH_ID;

	if (mrb_type(ret) == MRB_TT_FIXNUM)
		return (ECN_ENOD_ID)mrb_fixnum(ret);

	return ENOD_NOT_MATCH_ID;
}

ECN_ENOD_ID lcl_add_remote_addr(ecnl_svc_task_t *svc, T_EDATA *edata, mrb_value ep)
{
	mrb_state *mrb = svc->mrb;
	mrb_value edat = mrb_obj_value(edata);
	mrb_value ret;

	ret = mrb_funcall(mrb, svc->self, "add_remote_addr", 2, edat, ep);

	if (mrb_type(ret) == MRB_TT_FALSE)
		return ENOD_NOT_MATCH_ID;

	if (mrb_type(ret) == MRB_TT_FIXNUM)
		return (ECN_ENOD_ID)mrb_fixnum(ret);

	return ENOD_NOT_MATCH_ID;
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

	/* アドレスID登録なし */
	mrb_define_const(mrb, _module_ecnl, "ENOD_NOT_MATCH_ID", mrb_fixnum_value(ENOD_NOT_MATCH_ID));
	/* マルチキャストアドレスID */
	mrb_define_const(mrb, _module_ecnl, "ENOD_MULTICAST_ID", mrb_fixnum_value(ENOD_MULTICAST_ID));
	/* 自ノードアドレスID */
	mrb_define_const(mrb, _module_ecnl, "ENOD_LOCAL_ID", mrb_fixnum_value(ENOD_LOCAL_ID));
	/* APIアドレスID */
	mrb_define_const(mrb, _module_ecnl, "ENOD_API_ID", mrb_fixnum_value(ENOD_API_ID));
	/* 他ノードID */
	mrb_define_const(mrb, _module_ecnl, "ENOD_REMOTE_ID", mrb_fixnum_value(ENOD_REMOTE_ID));

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
	mrb_define_method(mrb, _class_property, "pcd", mrb_ecnl_eproperty_get_pcd, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティ属性 */
	mrb_define_method(mrb, _class_property, "atr", mrb_ecnl_eproperty_get_atr, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティのサイズ */
	mrb_define_method(mrb, _class_property, "sz", mrb_ecnl_eproperty_get_sz, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの拡張情報 */
	mrb_define_method(mrb, _class_property, "exinf", mrb_ecnl_eproperty_get_exinf, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_property, "set_exinf", mrb_ecnl_eproperty_set_exinf, MRB_ARGS_REQ(1));
	/* ECHONET Lite プロパティの設定関数 */
	mrb_define_method(mrb, _class_property, "setcb", mrb_ecnl_eproperty_get_setcb, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの取得関数 */
	mrb_define_method(mrb, _class_property, "getcb", mrb_ecnl_eproperty_get_getcb, MRB_ARGS_NONE());
	/* ECHONET Lite プロパティの通知有無 */
	mrb_define_method(mrb, _class_property, "anno", mrb_ecnl_eproperty_get_anno, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_property, "set_anno", mrb_ecnl_eproperty_set_anno, MRB_ARGS_REQ(1));

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
	mrb_define_method(mrb, _class_data, "esv", mrb_ecnl_get_esv, MRB_ARGS_NONE());

	/* 応答電文解析イテレーター初期化 */
	mrb_define_method(mrb, _class_data, "itr_ini", mrb_ecnl_itr_ini, MRB_ARGS_NONE());

	_class_iterator = mrb_define_class_under(mrb, _module_ecnl, "EIterator", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_iterator, MRB_TT_DATA);
	mrb_define_method(mrb, _class_iterator, "initialize", mrb_ecnl_eiterator_initialize, MRB_ARGS_NONE());

	/* 応答電文解析イテレーターインクリメント */
	mrb_define_method(mrb, _class_iterator, "itr_nxt", mrb_ecnl_itr_nxt, MRB_ARGS_NONE());

	mrb_define_method(mrb, _class_iterator, "epc", mrb_ecnl_iterator_get_epc, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_iterator, "edt", mrb_ecnl_iterator_get_edt, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_iterator, "state", mrb_ecnl_iterator_get_state, MRB_ARGS_NONE());
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
	mrb_define_method(mrb, _class_svctask, "set_timer", mrb_ecnl_svctask_set_timer, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, _class_svctask, "timer", mrb_ecnl_svctask_get_timer, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_svctask, "progress", mrb_ecnl_svctask_progress, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, _class_svctask, "recv_msg", mrb_ecnl_svctask_recv_msg, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, _class_svctask, "call_timeout", mrb_ecnl_svctask_call_timeout, MRB_ARGS_NONE());

	/* リモートECHONETノードの適合確認 */
	mrb_define_method(mrb, _class_svctask, "is_match", mrb_ecnl_svctask_is_match, MRB_ARGS_REQ(3));
}

void mrb_mruby_ecnl_gem_final(mrb_state *mrb)
{
}
