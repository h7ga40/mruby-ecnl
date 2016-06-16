/*
 *  TOPPERS ECHONET Lite Communication Middleware
 * 
 *  Copyright (C) 2016 Cores Co., Ltd. Japan
 * 
 *  ��L���쌠�҂́C�ȉ���(1)�`(4)�̏����𖞂����ꍇ�Ɍ���C�{�\�t�g�E�F
 *  �A�i�{�\�t�g�E�F�A�����ς������̂��܂ށD�ȉ������j���g�p�E�����E��
 *  �ρE�Ĕz�z�i�ȉ��C���p�ƌĂԁj���邱�Ƃ𖳏��ŋ�������D
 *  (1) �{�\�t�g�E�F�A���\�[�X�R�[�h�̌`�ŗ��p����ꍇ�ɂ́C��L�̒���
 *      ���\���C���̗��p��������щ��L�̖��ۏ؋K�肪�C���̂܂܂̌`�Ń\�[
 *      �X�R�[�h���Ɋ܂܂�Ă��邱�ƁD
 *  (2) �{�\�t�g�E�F�A���C���C�u�����`���ȂǁC���̃\�t�g�E�F�A�J���Ɏg
 *      �p�ł���`�ōĔz�z����ꍇ�ɂ́C�Ĕz�z�ɔ����h�L�������g�i���p
 *      �҃}�j���A���Ȃǁj�ɁC��L�̒��쌠�\���C���̗��p��������щ��L
 *      �̖��ۏ؋K����f�ڂ��邱�ƁD
 *  (3) �{�\�t�g�E�F�A���C�@��ɑg�ݍ��ނȂǁC���̃\�t�g�E�F�A�J���Ɏg
 *      �p�ł��Ȃ��`�ōĔz�z����ꍇ�ɂ́C���̂����ꂩ�̏����𖞂�����
 *      �ƁD
 *    (a) �Ĕz�z�ɔ����h�L�������g�i���p�҃}�j���A���Ȃǁj�ɁC��L�̒�
 *        �쌠�\���C���̗��p��������щ��L�̖��ۏ؋K����f�ڂ��邱�ƁD
 *    (b) �Ĕz�z�̌`�Ԃ��C�ʂɒ�߂���@�ɂ���āCTOPPERS�v���W�F�N�g��
 *        �񍐂��邱�ƁD
 *  (4) �{�\�t�g�E�F�A�̗��p�ɂ�蒼�ړI�܂��͊ԐړI�ɐ����邢���Ȃ鑹
 *      �Q������C��L���쌠�҂����TOPPERS�v���W�F�N�g��Ɛӂ��邱�ƁD
 *      �܂��C�{�\�t�g�E�F�A�̃��[�U�܂��̓G���h���[�U����̂����Ȃ闝
 *      �R�Ɋ�Â�����������C��L���쌠�҂����TOPPERS�v���W�F�N�g��
 *      �Ɛӂ��邱�ƁD
 * 
 *  �{�\�t�g�E�F�A�́C���ۏ؂Œ񋟂���Ă�����̂ł���D��L���쌠�҂�
 *  ���TOPPERS�v���W�F�N�g�́C�{�\�t�g�E�F�A�Ɋւ��āC����̎g�p�ړI
 *  �ɑ΂���K�������܂߂āC�����Ȃ�ۏ؂��s��Ȃ��D�܂��C�{�\�t�g�E�F
 *  �A�̗��p�ɂ�蒼�ړI�܂��͊ԐړI�ɐ����������Ȃ鑹�Q�Ɋւ��Ă��C��
 *  �̐ӔC�𕉂�Ȃ��D
 * 
 */

#ifndef MRB_ECNL_H
#define MRB_ECNL_H

#include <stddef.h>
#include <stdint.h>
#include <mruby.h>
#include "echonet.h"

extern struct RClass *_module_ecnl;
extern struct RClass *_class_object;
extern struct RClass *_class_node;
extern struct RClass *_class_property;
extern struct RClass *_class_data;
extern struct RClass *_class_iterator;
extern struct RClass *_class_svctask;

typedef struct mrb_ecnl_eproperty_type
{
	EPRPINIB inib;
	mrb_sym exinf;
	mrb_sym eprpset;
	mrb_sym eprpget;
	mrb_value eobj;
	bool_t anno;
}T_MRB_ECNL_EPROPERTY;

typedef struct mrb_ecnl_eiterator_type
{
	T_ENUM_EPC itr;
	uint8_t epc, pdc;
	uint8_t edt[256];
	uint8_t state : 7;
	uint8_t is_eof : 1;
}T_MRB_ECNL_EITERATOR;

mrb_value mrb_ecnl_edata_new(mrb_state *mrb, T_EDATA *data);

bool_t lcl_is_local_addr(ecnl_svc_task_t *svc, mrb_value ep);
bool_t lcl_is_multicast_addr(ecnl_svc_task_t *svc, mrb_value ep);
bool_t lcl_equals_addr(ecnl_svc_task_t *svc, mrb_value ep1, mrb_value ep2);
mrb_value lcl_get_local_addr(ecnl_svc_task_t *svc);
mrb_value lcl_get_multicast_addr(ecnl_svc_task_t *svc);
bool_t lcl_is_match(ecnl_svc_task_t *svc, struct ecn_node *enodcb, T_EDATA *edata, mrb_value ep);
ER lcl_snd_msg(ecnl_svc_task_t *svc, mrb_value ep, mrb_value msg);

void mrb_mruby_ecnl_gem_init(mrb_state *mrb);
void mrb_mruby_ecnl_gem_final(mrb_state *mrb);

#endif /* MRB_ECNL_H */
