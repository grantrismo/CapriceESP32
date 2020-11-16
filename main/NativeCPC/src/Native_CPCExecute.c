/*
    CaPriCe for Palm OS - Amstrad CPC 464/664/6128 emulator for Palm devices
    Copyright (C) 2009  Fr�d�ric Coste

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>

#ifdef __PALMOS__
#include <ByteOrderUtils.h>
#endif
#include "../common/types.h"
#include "../common/trace_data.h"

//#ifndef SIM
//#include "esp_attr.h"
//#endif

// Copied from Window.h
typedef tUChar IndexedColorType;      // 1, 2, 4, or 8 bit index


#include "Native_CPC.h"

//
// CAUTION:
// CPC Engine patches must be confirmed on TE !!
// TX allows code optimization TE doesn't accept !!
//

//===================
// PATCH begin
#ifdef _PATCH_ENABLE

#undef PATCH_UCHAR
//#define PATCH_UCHAR // Remove (tUChar) explicite cast
#undef PATCH_USHORT
//#define PATCH_USHORT // Remove (tUShort) explicite cast
#undef PATCH_INLINE
//#define PATCH_INLINE // Set function as inline

#endif /* _PATCH_ENABLE */
// PATCH end
//===================


/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __PALMOS__
tULong PNOMain(const tVoid*,
               tVoid*,
               Call68KFuncType*);
#endif

#ifndef PATCH_UCHAR
#define CAST_UCHAR(x) ((tUChar)(x))
#else /* PATCH_UCHAR */
#define CAST_UCHAR(x) (x)
#endif /* PATCH_UCHAR */

#ifndef PATCH_USHORT
#define CAST_USHORT(x) ((tUShort)(x))
#else /* PATCH_USHORT */
#define CAST_USHORT(x) (x)
#endif /* PATCH_USHORT */

#ifdef PATCH_INLINE
#define INLINE_FCT inline
#else /* PATCH_INLINE */
#define INLINE_FCT
#endif /* PATCH_INLINE */

#ifdef SIM
#define ATTR_EXTRA
#else
#include "esp_attr.h"
#define ATTR_EXTRA //IRAM_ATTR
#endif


// copied from ErrorBase.h
#define errNone                       0x0000  // No error

// generic
static inline tUChar read_mem(tNativeCPC* NativeCPC, tULong addr);
static inline tVoid write_mem(tNativeCPC* NativeCPC, tULong addr, tULong val);
static inline tChar signed_read_mem(tNativeCPC* NativeCPC, tULong addr);

// Z80
static tVoid z80_pfx_cb(tNativeCPC* NativeCPC);
static tVoid z80_pfx_dd(tNativeCPC* NativeCPC);
static tVoid z80_pfx_ddcb(tNativeCPC* NativeCPC);
static tVoid z80_pfx_ed(tNativeCPC* NativeCPC);
static tVoid z80_pfx_fd(tNativeCPC* NativeCPC);
static tVoid z80_pfx_fdcb(tNativeCPC* NativeCPC);

// CPC
static tULong z80_IN_handler(tNativeCPC* NativeCPC,
                             tRegister port);
static tVoid  z80_OUT_handler(tNativeCPC* NativeCPC,
                              tRegister port,
                              tULong val);

// Gate Array
static tVoid ga_memory_manager(tNativeCPC* NativeCPC);
static tVoid ga_init_banking(tNativeCPC* NativeCPC);

// Video
static tVoid video_init(tNativeCPC* NativeCPC);
static tVoid video_access_memory(tNativeCPC* NativeCPC,
                                 tULong repeat_count);
static tVoid video_draw_mode0(tVDU* VDU,
                              tULong addr);
static tVoid video_draw_mode1(tVDU* VDU,
                              tULong addr);
static tVoid video_draw_mode2(tVDU* VDU,
                              tULong addr);
static tVoid video_draw_mode2_antialiased(tVDU* VDU,
                                          tULong addr);

// Audio
static tVoid audio_init(tNativeCPC* NativeCPC);
static inline tVoid audio_set_AY_Register(tPSG* PSG,
                                          tULong Num,
                                          tULong Value);
static inline tVoid audio_set_envelope_register(tPSG* PSG,
                                                tULong Value);
static tVoid audio_Case_EnvType_0_3__9(tPSG* PSG);
static tVoid audio_Case_EnvType_4_7__15(tPSG* PSG);
static tVoid audio_Case_EnvType_8(tPSG* PSG);
static tVoid audio_Case_EnvType_10(tPSG* PSG);
static tVoid audio_Case_EnvType_11(tPSG* PSG);
static tVoid audio_Case_EnvType_12(tPSG* PSG);
static tVoid audio_Case_EnvType_13(tPSG* PSG);
static tVoid audio_Case_EnvType_14(tPSG* PSG);
static tVoid audio_Synthesizer_Logic_Q(tPSG* PSG);
#if SND_STEREO == 1
static tVoid audio_Synthesizer_Mixer_Q(tNativeCPC* NativeCPC);
#  if SND_16BITS == 1
static tVoid ATTR_EXTRA audio_Synthesizer_Stereo16(tNativeCPC* NativeCPC);
#  else /* SND_16BITS == 1 */
static tVoid ATTR_EXTRA audio_Synthesizer_Stereo8(tNativeCPC* NativeCPC);
#  endif /* SND_16BITS == 1 */
#else /* SND_STEREO == 1 */
static tVoid audio_Synthesizer_Mixer_Q_Mono(tNativeCPC* NativeCPC);
static tVoid ATTR_EXTRA audio_Synthesizer_Mono8(tNativeCPC* NativeCPC);
#endif /* SND_STEREO == 1 */

// FDC
static tVoid fdc_init(tNativeCPC* NativeCPC);
static tULong fdc_read_status(tFDC* FDC);
static tULong fdc_read_data(tFDC* FDC);
static tVoid fdc_write_data(tNativeCPC* NativeCPC,
                            tULong val);
static tVoid fdc_specify(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_drvstat(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_recalib(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_intstat(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_seek(tFDC* FDC,
                      tNativeCPC* NativeCPC);
static tVoid fdc_readtrk(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_write(tFDC* FDC,
                       tNativeCPC* NativeCPC);
static tVoid fdc_read(tFDC* FDC,
                      tNativeCPC* NativeCPC);
static tVoid fdc_readID(tFDC* FDC,
                        tNativeCPC* NativeCPC);
static tVoid fdc_writeID(tFDC* FDC,
                         tNativeCPC* NativeCPC);
static tVoid fdc_scan(tFDC* FDC,
                      tNativeCPC* NativeCPC);
static tVoid fdc_cmd_readtrk(tFDC* FDC);
static tVoid fdc_cmd_read(tFDC* FDC);
static tSector* fdc_find_sector(tFDC* FDC,
                                tULong* requested_CHRN);
static tVoid fdc_cmd_scan(tFDC* FDC);
static tVoid fdc_cmd_write(tFDC* FDC);
static tULong fdc_init_status_regs(tFDC* FDC);

// Tape
#ifdef ENABLE_TAPE
static tLong Tape_ReadDataBit(tNativeCPC* NativeCPC);
static tLong Tape_ReadSampleDataBit(tNativeCPC* NativeCPC);
static tVoid Tape_BlockDone(tNativeCPC* NativeCPC);
static tVoid Tape_UpdateLevel(tNativeCPC* NativeCPC);
#endif /* ENABLE_TAPE */


//
// Routines
//
static inline tVoid  MemMoveByte(tUChar* destP,
                                 tUChar* sourceP,
                                 tULong numBytes);
static inline tVoid  MemSetByte(tUChar* destP,
                                tULong numBytes,
                                tULong value);
static inline tVoid  MemSetLong(tULong* destP,
                                tULong numLongs,
                                tULong value);


//
// TestU
//
#ifdef _TESTU
static tUShort PerformTestU(tNativeCPC* NativeCPC);
#endif /* _TESTU */



enum opcodes
{
  /*00*/ nop, ld_bc_word, ld_mbc_a, inc_bc, inc_b, dec_b, ld_b_byte, rlca,
  /*08*/ ex_af_af, add_hl_bc, ld_a_mbc, dec_bc, inc_c, dec_c, ld_c_byte, rrca,
  /*10*/ djnz, ld_de_word, ld_mde_a, inc_de, inc_d, dec_d, ld_d_byte, rla,
  /*18*/ jr, add_hl_de, ld_a_mde, dec_de, inc_e, dec_e, ld_e_byte, rra,
  /*20*/ jr_nz, ld_hl_word, ld_mword_hl, inc_hl, inc_h, dec_h, ld_h_byte, daa,
  /*28*/ jr_z, add_hl_hl, ld_hl_mword, dec_hl, inc_l, dec_l, ld_l_byte, cpl,
  /*30*/ jr_nc, ld_sp_word, ld_mword_a, inc_sp, inc_mhl, dec_mhl, ld_mhl_byte, scf,
  /*38*/ jr_c, add_hl_sp, ld_a_mword, dec_sp, inc_a, dec_a, ld_a_byte, ccf,
  /*40*/ ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_mhl, ld_b_a,
  /*48*/ ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_mhl, ld_c_a,
  /*50*/ ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_mhl, ld_d_a,
  /*58*/ ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_mhl, ld_e_a,
  /*60*/ ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_mhl, ld_h_a,
  /*68*/ ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_mhl, ld_l_a,
  /*70*/ ld_mhl_b, ld_mhl_c, ld_mhl_d, ld_mhl_e, ld_mhl_h, ld_mhl_l, halt, ld_mhl_a,
  /*78*/ ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_mhl, ld_a_a,
  /*80*/ add_b, add_c, add_d, add_e, add_h, add_l, add_mhl, add_a,
  /*88*/ adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_mhl, adc_a,
  /*90*/ sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_mhl, sub_a,
  /*98*/ sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_mhl, sbc_a,
  /*a0*/ and_b, and_c, and_d, and_e, and_h, and_l, and_mhl, and_a,
  /*a8*/ xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_mhl, xor_a,
  /*b0*/ or_b, or_c, or_d, or_e, or_h, or_l, or_mhl, or_a,
  /*b8*/ cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_mhl, cp_a,
  /*c0*/ ret_nz, pop_bc, jp_nz, jp, call_nz, push_bc, add_byte, rst00,
  /*c8*/ ret_z, ret, jp_z, pfx_cb, call_z, call, adc_byte, rst08,
  /*d0*/ ret_nc, pop_de, jp_nc, outa, call_nc, push_de, sub_byte, rst10,
  /*d8*/ ret_c, exx, jp_c, ina, call_c, pfx_dd, sbc_byte, rst18,
  /*e0*/ ret_po, pop_hl, jp_po, ex_msp_hl, call_po, push_hl, and_byte, rst20,
  /*e8*/ ret_pe, ld_pc_hl, jp_pe, ex_de_hl, call_pe, pfx_ed, xor_byte, rst28,
  /*f0*/ ret_p, pop_af, jp_p, di, call_p, push_af, or_byte, rst30,
  /*f8*/ ret_m, ld_sp_hl, jp_m, ei, call_m, pfx_fd, cp_byte, rst38
};

enum CBcodes
{
  /*00*/ rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_mhl, rlc_a,
  /*08*/ rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_mhl, rrc_a,
  /*10*/ rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_mhl, rl_a,
  /*18*/ rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_mhl, rr_a,
  /*20*/ sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_mhl, sla_a,
  /*28*/ sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_mhl, sra_a,
  /*30*/ sll_b, sll_c, sll_d, sll_e, sll_h, sll_l, sll_mhl, sll_a,
  /*38*/ srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_mhl, srl_a,
  /*40*/ bit0_b, bit0_c, bit0_d, bit0_e, bit0_h, bit0_l, bit0_mhl, bit0_a,
  /*48*/ bit1_b, bit1_c, bit1_d, bit1_e, bit1_h, bit1_l, bit1_mhl, bit1_a,
  /*50*/ bit2_b, bit2_c, bit2_d, bit2_e, bit2_h, bit2_l, bit2_mhl, bit2_a,
  /*58*/ bit3_b, bit3_c, bit3_d, bit3_e, bit3_h, bit3_l, bit3_mhl, bit3_a,
  /*60*/ bit4_b, bit4_c, bit4_d, bit4_e, bit4_h, bit4_l, bit4_mhl, bit4_a,
  /*68*/ bit5_b, bit5_c, bit5_d, bit5_e, bit5_h, bit5_l, bit5_mhl, bit5_a,
  /*70*/ bit6_b, bit6_c, bit6_d, bit6_e, bit6_h, bit6_l, bit6_mhl, bit6_a,
  /*78*/ bit7_b, bit7_c, bit7_d, bit7_e, bit7_h, bit7_l, bit7_mhl, bit7_a,
  /*80*/ res0_b, res0_c, res0_d, res0_e, res0_h, res0_l, res0_mhl, res0_a,
  /*88*/ res1_b, res1_c, res1_d, res1_e, res1_h, res1_l, res1_mhl, res1_a,
  /*90*/ res2_b, res2_c, res2_d, res2_e, res2_h, res2_l, res2_mhl, res2_a,
  /*98*/ res3_b, res3_c, res3_d, res3_e, res3_h, res3_l, res3_mhl, res3_a,
  /*a0*/ res4_b, res4_c, res4_d, res4_e, res4_h, res4_l, res4_mhl, res4_a,
  /*a8*/ res5_b, res5_c, res5_d, res5_e, res5_h, res5_l, res5_mhl, res5_a,
  /*b0*/ res6_b, res6_c, res6_d, res6_e, res6_h, res6_l, res6_mhl, res6_a,
  /*b8*/ res7_b, res7_c, res7_d, res7_e, res7_h, res7_l, res7_mhl, res7_a,
  /*c0*/ set0_b, set0_c, set0_d, set0_e, set0_h, set0_l, set0_mhl, set0_a,
  /*c8*/ set1_b, set1_c, set1_d, set1_e, set1_h, set1_l, set1_mhl, set1_a,
  /*d0*/ set2_b, set2_c, set2_d, set2_e, set2_h, set2_l, set2_mhl, set2_a,
  /*d8*/ set3_b, set3_c, set3_d, set3_e, set3_h, set3_l, set3_mhl, set3_a,
  /*e0*/ set4_b, set4_c, set4_d, set4_e, set4_h, set4_l, set4_mhl, set4_a,
  /*e8*/ set5_b, set5_c, set5_d, set5_e, set5_h, set5_l, set5_mhl, set5_a,
  /*f0*/ set6_b, set6_c, set6_d, set6_e, set6_h, set6_l, set6_mhl, set6_a,
  /*f8*/ set7_b, set7_c, set7_d, set7_e, set7_h, set7_l, set7_mhl, set7_a
};

enum EDcodes
{
  /*00*/ ed_00, ed_01, ed_02, ed_03, ed_04, ed_05, ed_06, ed_07,
  /*08*/ ed_08, ed_09, ed_0a, ed_0b, ed_0c, ed_0d, ed_0e, ed_0f,
  /*10*/ ed_10, ed_11, ed_12, ed_13, ed_14, ed_15, ed_16, ed_17,
  /*18*/ ed_18, ed_19, ed_1a, ed_1b, ed_1c, ed_1d, ed_1e, ed_1f,
  /*20*/ ed_20, ed_21, ed_22, ed_23, ed_24, ed_25, ed_26, ed_27,
  /*28*/ ed_28, ed_29, ed_2a, ed_2b, ed_2c, ed_2d, ed_2e, ed_2f,
  /*30*/ ed_30, ed_31, ed_32, ed_33, ed_34, ed_35, ed_36, ed_37,
  /*38*/ ed_38, ed_39, ed_3a, ed_3b, ed_3c, ed_3d, ed_3e, ed_3f,
  /*40*/ in_b_c, out_c_b, sbc_hl_bc, ld_EDmword_bc, neg, retn, im_0, ld_i_a,
  /*48*/ in_c_c, out_c_c, adc_hl_bc, ld_EDbc_mword, neg_1, reti, im_0_1, ld_r_a,
  /*50*/ in_d_c, out_c_d, sbc_hl_de, ld_EDmword_de, neg_2, retn_1, im_1, ld_a_i,
  /*58*/ in_e_c, out_c_e, adc_hl_de, ld_EDde_mword, neg_3, reti_1, im_2, ld_a_r,
  /*60*/ in_h_c, out_c_h, sbc_hl_hl, ld_EDmword_hl, neg_4, retn_2, im_0_2, rrd,
  /*68*/ in_l_c, out_c_l, adc_hl_hl, ld_EDhl_mword, neg_5, reti_2, im_0_3, rld,
  /*70*/ in_0_c, out_c_0, sbc_hl_sp, ld_EDmword_sp, neg_6, retn_3, im_1_1, ed_77,
  /*78*/ in_a_c, out_c_a, adc_hl_sp, ld_EDsp_mword, neg_7, reti_3, im_2_1, ed_7f,
  /*80*/ ed_80, ed_81, ed_82, ed_83, ed_84, ed_85, ed_86, ed_87,
  /*88*/ ed_88, ed_89, ed_8a, ed_8b, ed_8c, ed_8d, ed_8e, ed_8f,
  /*90*/ ed_90, ed_91, ed_92, ed_93, ed_94, ed_95, ed_96, ed_97,
  /*98*/ ed_98, ed_99, ed_9a, ed_9b, ed_9c, ed_9d, ed_9e, ed_9f,
  /*a0*/ ldi, cpi, ini, outi, ed_a4, ed_a5, ed_a6, ed_a7,
  /*a8*/ ldd, cpd, ind, outd, ed_ac, ed_ad, ed_ae, ed_af,
  /*b0*/ ldir, cpir, inir, otir, ed_b4, ed_b5, ed_b6, ed_b7,
  /*b8*/ lddr, cpdr, indr, otdr, ed_bc, ed_bd, ed_be, ed_bf,
  /*c0*/ ed_c0, ed_c1, ed_c2, ed_c3, ed_c4, ed_c5, ed_c6, ed_c7,
  /*c8*/ ed_c8, ed_c9, ed_ca, ed_cb, ed_cc, ed_cd, ed_ce, ed_cf,
  /*d0*/ ed_d0, ed_d1, ed_d2, ed_d3, ed_d4, ed_d5, ed_d6, ed_d7,
  /*d8*/ ed_d8, ed_d9, ed_da, ed_db, ed_dc, ed_dd, ed_de, ed_df,
  /*e0*/ ed_e0, ed_e1, ed_e2, ed_e3, ed_e4, ed_e5, ed_e6, ed_e7,
  /*e8*/ ed_e8, ed_e9, ed_ea, ed_eb, ed_ec, ed_ed, ed_ee, ed_ef,
  /*f0*/ ed_f0, ed_f1, ed_f2, ed_f3, ed_f4, ed_f5, ed_f6, ed_f7,
  /*f8*/ ed_f8, ed_f9, ed_fa, ed_fb, ed_fc, ed_fd, ed_fe, ed_ff
};


static tUChar irep_tmp1[4][4] =
{
   {0, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 0}
};

/* tmp1 value for ind/indr/outd/otdr for [C.1-0][io.1-0] */
static tUChar drep_tmp1[4][4] =
{
   {0, 1, 0, 0}, {1, 0, 0, 1}, {0, 0, 1, 0}, {0, 1, 0, 1}
};

/* tmp2 value for all in/out repeated opcodes for B.7-0 */
static tUChar breg_tmp2[256] =
{
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1
};

#define Oa 8
#define Oa_ 4
#define Ia 12
#define Ia_ 0

static tUChar cc_op[256] =
{
    4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
   12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
    8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
    8, 12, 16,  8, 12, 12, 12,  4,  8, 12, 16,  8,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
    8, 12, 12, Oa, 12, 16,  8, 16,  8,  4, 12, Ia, 12,  4,  8, 16,
    8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
    8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static tUChar cc_cb[256] =
{
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4
};

#define Ox 8
#define Ox_ 4
#define Oy 12
#define Oy_ 4
#define Ix 12
#define Ix_ 0
#define Iy 16
#define Iy_ 0

static tUChar cc_ed[256] =
{
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
   Ix, Ox, 12, 20,  4, 12,  4,  8, Ix, Ox, 12, 20,  4, 12,  4,  8,
   Ix, Ox, 12, 20,  4, 12,  4,  8, Ix, Ox, 12, 20,  4, 12,  4,  8,
   Ix, Ox, 12, 20,  4, 12,  4, 16, Ix, Ox, 12, 20,  4, 12,  4, 16,
   Ix, Ox, 12, 20,  4, 12,  4,  4, Ix, Ox, 12, 20,  4, 12,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
   16, 12, Iy, Oy,  4,  4,  4,  4, 16, 12, Iy, Oy,  4,  4,  4,  4,
   16, 12, Iy, Oy,  4,  4,  4,  4, 16, 12, Iy, Oy,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
};

static tUChar cc_xy[256] =
{
    4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
   12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
    8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
    8, 12, 16,  8, 20, 20, 20,  4,  8, 12, 16,  8,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
   16, 16, 16, 16, 16, 16,  4, 16,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
    8, 12, 12, Oa, 12, 16,  8, 16,  8,  4, 12, Ia, 12,  4,  8, 16,
    8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
    8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static tUChar cc_xycb[256] =
{
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

static tUChar cc_ex[256] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  8,  4,  4,  0,  0,  0,  0,  4,  8,  4,  4,  0,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0
};



#if SND_STEREO == 1
#  if SND_16BITS == 1
#    define  Z80_WAIT_STATES_PSG_SYNTHESIZER  audio_Synthesizer_Stereo16(NativeCPC);
#  else /* SND_16BITS == 1 */
#    define  Z80_WAIT_STATES_PSG_SYNTHESIZER  audio_Synthesizer_Stereo8(NativeCPC);
#  endif /* SND_16BITS == 1 */
#else /* SND_STEREO == 1 */
#  define  Z80_WAIT_STATES_PSG_SYNTHESIZER  audio_Synthesizer_Mono8(NativeCPC);
#endif /* SND_STEREO == 1 */


#ifdef _DEBUG
#  define PSG_SAMPLECOUNT     PSG->SampleCount++;
#else /* _DEBUG */
#  define PSG_SAMPLECOUNT
#endif /* _DEBUG */


#ifdef ENABLE_TAPE
#  define Z80_WAIT_STATES_TAPE \
  if ( (NativeCPC->tape_motor) && (NativeCPC->tape_play_button) ) \
  { \
    NativeCPC->iTapeCycleCount -= Z80->iCycleCount; \
    if (NativeCPC->iTapeCycleCount <= 0) \
    { \
      Tape_UpdateLevel(NativeCPC); \
    } \
  }
#else /* ENABLE_TAPE */
#  define Z80_WAIT_STATES_TAPE
#endif /* ENABLE_TAPE */


#define Z80_WAIT_STATES_AUDIO \
  if (PSG->snd_enabled) \
  { \
    PSG->cycle_count += Z80->iCycleCount << 16; \
    if (PSG->cycle_count >= SND_CYCLE_SOUND_INIT) \
    { \
      PSG->cycle_count -= SND_CYCLE_SOUND_INIT; \
      Z80_WAIT_STATES_PSG_SYNTHESIZER; \
      PSG_SAMPLECOUNT; \
    } \
  }

#define Z80_WAIT_STATES_FDC \
  if (FDC->phase == EXEC_PHASE) \
  { \
    FDC->timeout -= Z80->iCycleCount; \
    if (FDC->timeout <= 0) \
    { \
      PROFILE_ADD_NATIVE(PROFILE_fdc_overrun); \
      FDC->flags |= OVERRUN_flag; \
      if (FDC->cmd_direction == FDC_TO_CPU) \
      { \
        fdc_read_data(FDC); \
      } \
      else \
      { \
        fdc_write_data(NativeCPC, \
                       0xff); \
      } \
    } \
  }


#define Z80_WAIT_STATES \
{ \
  PROFILE_ADD_NATIVE(PROFILE_Z80_WAIT_STATES); \
  \
  if (Z80->iCycleCount) \
  { \
    /* Video */ \
    video_access_memory(NativeCPC, \
                        Z80->iCycleCount >> 2); \
    \
    /* Audio */ \
    Z80_WAIT_STATES_AUDIO; \
    \
    /* FDC */ \
    Z80_WAIT_STATES_FDC; \
    \
    /* TAPE */ \
    Z80_WAIT_STATES_TAPE; \
    \
    NativeCPC->cycle_count -= Z80->iCycleCount; \
  } \
}


#define AUDIO_SET_CASE_ENV(v) \
switch (v) \
{ \
  case 0: \
  case 1: \
  case 2: \
  case 3: \
  case 9: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_0_3__9; \
    break; \
  case 4: \
  case 5: \
  case 6: \
  case 7: \
  case 15: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_4_7__15; \
    break; \
  case 8: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_8; \
    break; \
  case 10: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_10; \
    break; \
  case 11: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_11; \
    break; \
  case 12: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_12; \
    break; \
  case 13: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_13; \
    break; \
  case 14: \
    PSG->Case_Env = (tCaseEnvType)audio_Case_EnvType_14; \
    break; \
}


// ==================================================================================
//
// Z80 Instruction macros
//
// ==================================================================================

#define ADC(value) \
{ \
tULong val = value; \
tULong res = _A + val + (_F & Cflag); \
  _F = Z80->SZ[(tUChar)res] | \
       ((res >> 8) & Cflag) | \
       ((_A ^ res ^ val) & Hflag) | \
       (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
  _A = res; \
}

#define ADD(value) \
{ \
tULong val = value; \
tULong res = _A + val; \
  _F = Z80->SZ[(tUChar)res] | \
       ((res >> 8) & Cflag) | \
       ((_A ^ res ^ val) & Hflag) | \
       (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
  _A = res; \
}

#define ADD16(dest, src) \
{ \
tULong res = Z80->Regs.dest.d + Z80->Regs.src.d; \
  _F = (_F & (Sflag | Zflag | Vflag)) | \
       (((Z80->Regs.dest.d ^ res ^ Z80->Regs.src.d) >> 8) & Hflag) | \
       ((res >> 16) & Cflag) | \
       ((res >> 8) & Xflags); \
  Z80->Regs.dest.w.l = res; \
}

#define AND(val) \
{ \
  _A &= val; \
  _F = Z80->SZP[_A] | Hflag; \
}

#define CALL \
{ \
tRegister dest; \
  dest.b.l = read_mem(NativeCPC, \
                      _PC++); /* subroutine address low byte */ \
  dest.b.h = read_mem(NativeCPC, \
                      _PC++); /* subroutine address high byte */ \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.PC.b.h); /* store high byte of current PC */ \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.PC.b.l); /* store low byte of current PC */ \
  _PC = dest.w.l; /* continue execution at subroutine */ \
}

#define CP(value) \
{ \
tULong val = value; \
tULong res = _A - val; \
  _F = (Z80->SZ[(tUChar)res] & (Sflag | Zflag)) | \
       (val & Xflags) | \
       ((res >> 8) & Cflag) | \
       Nflag | \
       ((_A ^ res ^ val) & Hflag) | \
       ((((val ^ _A) & (_A ^ res)) >> 5) & Vflag); \
}

#define DAA \
{ \
tULong idx = _A; \
  if (_F & Cflag) \
    idx |= 0x100; \
  if (_F & Hflag) \
    idx |= 0x200; \
  if (_F & Nflag) \
    idx |= 0x400; \
  _AF = NativeCPC->DAATable[idx]; \
}

#define DEC(reg) \
{ \
  reg--; \
  _F = (_F & Cflag) | \
       Z80->SZHV_dec[(tUChar)reg]; \
}

#define JR \
{ \
tLong offset = signed_read_mem(NativeCPC, \
                               _PC); /* grab signed jump offset */ \
  _PC += offset + 1; /* add offset & correct PC */ \
}

#define EXX \
{ \
tRegister temp = Z80->Regs.BCx; \
  Z80->Regs.BCx = Z80->Regs.BC; \
  Z80->Regs.BC = temp; \
  temp = Z80->Regs.DEx; \
  Z80->Regs.DEx = Z80->Regs.DE; \
  Z80->Regs.DE = temp; \
  temp = Z80->Regs.HLx; \
  Z80->Regs.HLx = Z80->Regs.HL; \
  Z80->Regs.HL = temp; \
}

#define EX(op1, op2) \
{ \
tRegister temp = op1; \
  op1 = op2; \
  op2 = temp; \
}

#define EX_SP(reg) \
{ \
tRegister temp; \
  temp.b.l = read_mem(NativeCPC, \
                      _SP++); \
  temp.b.h = read_mem(NativeCPC, \
                      _SP); \
  write_mem(NativeCPC, \
            _SP--, \
            Z80->Regs.reg.b.h); \
  write_mem(NativeCPC, \
            _SP, \
            Z80->Regs.reg.b.l); \
  Z80->Regs.reg.w.l = temp.w.l; \
}

#define INC(reg) \
{ \
  reg++; \
  _F = (_F & Cflag) | \
       Z80->SZHV_inc[(tUChar)reg]; \
}

#define JP \
{ \
tRegister addr; \
  addr.b.l = read_mem(NativeCPC, \
                      _PC++); \
  addr.b.h = read_mem(NativeCPC, \
                      _PC); \
  _PC = addr.w.l; \
}

#define LD16_MEM(reg) \
{ \
tRegister addr; \
  addr.b.l = read_mem(NativeCPC, \
                      _PC++); \
  addr.b.h = read_mem(NativeCPC, \
                      _PC++); \
  Z80->Regs.reg.b.l = read_mem(NativeCPC, \
                               addr.w.l); \
  Z80->Regs.reg.b.h = read_mem(NativeCPC, \
                               addr.w.l+1); \
}

#define LDMEM_16(reg) \
{ \
tRegister addr; \
  addr.b.l = read_mem(NativeCPC, \
                      _PC++); \
  addr.b.h = read_mem(NativeCPC, \
                      _PC++); \
  write_mem(NativeCPC, \
            addr.w.l, \
            Z80->Regs.reg.b.l); \
  write_mem(NativeCPC, \
            addr.w.l+1, \
            Z80->Regs.reg.b.h); \
}

#define OR(val) \
{ \
  _A |= val; \
  _F = Z80->SZP[_A]; \
}

#define POP(reg) \
{ \
  Z80->Regs.reg.b.l = read_mem(NativeCPC, \
                               _SP++); \
  Z80->Regs.reg.b.h = read_mem(NativeCPC, \
                               _SP++); \
}

#define PUSH(reg) \
{ \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.reg.b.h); \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.reg.b.l); \
}

#define RET \
{ \
  Z80->Regs.PC.b.l = read_mem(NativeCPC, \
                              _SP++); \
  Z80->Regs.PC.b.h = read_mem(NativeCPC, \
                              _SP++); \
}

#define RLA \
{ \
tULong res = (_A << 1) | (_F & Cflag); \
  _F = (_F & (Sflag | Zflag | Pflag)) | \
       ((_A & 0x80) >> 7) | \
       (res & Xflags); \
  _A = res; \
}

#define RLCA \
{ \
  _A = (_A << 1) | (_A >> 7); \
  _F = (_F & (Sflag | Zflag | Pflag)) | \
       (_A & (Xflags | Cflag)); \
}

#define RRA \
{ \
tULong res = (_A >> 1) | (_F << 7); \
  _F = (_F & (Sflag | Zflag | Pflag)) | \
       (_A & 0x01) | \
       (res & Xflags); \
  _A = res; \
}

#define RRCA \
{ \
  _F = (_F & (Sflag | Zflag | Pflag)) | \
       (_A & Cflag); \
  _A = (_A >> 1) | (_A << 7); \
  _F |= (_A & Xflags); \
}

#define RST(addr) \
{ \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.PC.b.h); /* store high byte of current PC */ \
  write_mem(NativeCPC, \
            --_SP, \
            Z80->Regs.PC.b.l); /* store low byte of current PC */ \
  _PC = addr; /* continue execution at restart address */ \
}

#define SBC(value) \
{ \
tULong val = value; \
tULong res = _A - val - (_F & Cflag); \
  _F = Z80->SZ[(tUChar)res] | \
       ((res >> 8) & Cflag) | \
       Nflag | \
       ((_A ^ res ^ val) & Hflag) | \
       (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
  _A = res; \
}

#define SUB(value) \
{ \
tULong val = value; \
tULong res = _A - val; \
  _F = Z80->SZ[(tUChar)res] | \
       ((res >> 8) & Cflag) | \
       Nflag | \
       ((_A ^ res ^ val) & Hflag) | \
       (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
  _A = res; \
}

#define XOR(val) \
{ \
  _A ^= val; \
  _F = Z80->SZP[_A]; \
}

#define BIT(bit, reg) \
{ \
  _F = (_F & Cflag) | \
       Hflag | \
       Z80->SZ_BIT[reg & (1 << bit)]; \
}

#define BIT_XY BIT

static inline tULong RES(tULong bit,
                  tULong val)
{
  return (val & ~(1 << bit));
}

static inline tULong RLC(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = (res & 0x80) >> 7; // Cflag

  res = ((res << 1) | (res >> 7)) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong RL(tZ80* Z80,
                 tULong val)
{
tULong res = val;
tULong carry = (res & 0x80) >> 7; // Cflag

  res = ((res << 1) | (_F & Cflag)) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong RRC(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = res & Cflag; // Cflag

  res = ((res >> 1) | (res << 7)) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong RR(tZ80* Z80,
                 tULong val)
{
tULong res = val;
tULong carry = res & Cflag; // Cflag

  res = ((res >> 1) | (_F << 7)) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong SET(tULong bit,
                  tULong val)
{
  return (val | (1 << bit));
}

static inline tULong SLA(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = (res & 0x80) >> 7; // Cflag

  res = (res << 1) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong SLL(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = (res & 0x80) >> 7; // Cflag

  res = ((res << 1) | 0x01) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong SRA(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = res & Cflag;

  res = ((res >> 1) | (res & 0x80)) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}

static inline tULong SRL(tZ80* Z80,
                  tULong val)
{
tULong res = val;
tULong carry = res & Cflag;

  res = (res >> 1) & 0xff;
  _F = Z80->SZP[res] |
       carry;

  return (res);
}


#define ADC16(reg) \
{ \
tULong res = _HLdword + Z80->Regs.reg.d + (_F & Cflag); \
  _F = (((_HLdword ^ res ^ Z80->Regs.reg.d) >> 8) & Hflag) | \
       ((res >> 16) & Cflag) | \
       ((res >> 8) & (Sflag | Xflags)) | \
       ((res & 0xffff) ? 0 : Zflag) | \
       (((Z80->Regs.reg.d ^ _HLdword ^ 0x8000) & (Z80->Regs.reg.d ^ res) & 0x8000) >> 13); \
  _HL = res; \
}

#define CPD \
{ \
tULong val = read_mem(NativeCPC, \
                      _HL); \
tULong res = _A - val; \
  _HL--; \
  _BC--; \
  _F = (_F & Cflag) | (Z80->SZ[(tUChar)res] & ~Xflags) | ((_A ^ val ^ res) & Hflag) | Nflag; \
  if (_F & Hflag) res -= 1; \
  _F |= res & Xflags; \
  if (_BC) _F |= Vflag; \
}

#define CPDR \
{ \
  CPD; \
  if (_BC && !(_F & Zflag)) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
    Z80->iWSAdjust++; \
  } \
}

#define CPI \
{ \
tULong val = read_mem(NativeCPC, \
                      _HL); \
tULong res = _A - val; \
  _HL++; \
  _BC--; \
  _F = (_F & Cflag) | (Z80->SZ[(tUChar)res] & ~Xflags) | ((_A ^ val ^ res) & Hflag) | Nflag; \
  if (_F & Hflag) res -= 1; \
  _F |= res & Xflags; \
  if (_BC) _F |= Vflag; \
}


#define CPIR \
{ \
  CPI; \
  if (_BC && !(_F & Zflag)) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
    Z80->iWSAdjust++; \
  } \
}

#define IND \
{ \
tULong io = z80_IN_handler(NativeCPC, \
                           Z80->Regs.BC); \
  _B--; \
  write_mem(NativeCPC, _HL, io); \
  _HL--; \
  _F = Z80->SZ[_B]; \
  if (io & Sflag) _F |= Nflag; \
  if ((((_C - 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
  if ((drep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
  { \
    _F |= Pflag; \
  } \
}

#define INDR \
{ \
  IND; \
  if (_B) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define INI \
{ \
tULong io = z80_IN_handler(NativeCPC, \
                           Z80->Regs.BC); \
   _B--; \
   write_mem(NativeCPC, \
             _HL, \
             io); \
   _HL++; \
   _F = Z80->SZ[_B]; \
   if(io & Sflag) _F |= Nflag; \
   if((((_C + 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
   if((irep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
      _F |= Pflag; \
}

#define INIR \
{ \
  INI; \
  if(_B) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define LDD \
{ \
tULong io = read_mem(NativeCPC, \
                     _HL); \
  write_mem(NativeCPC, \
            _DE, \
            io); \
  _F &= Sflag | Zflag | Cflag; \
  _F |= (_A + io) & Xflags; \
  _HL--; \
  _DE--; \
  _BC--; \
  if(_BC) _F |= Vflag; \
}

#define LDDR \
{ \
  LDD; \
  if(_BC) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define LDI \
{ \
tULong io = read_mem(NativeCPC, \
                     _HL); \
  write_mem(NativeCPC, \
            _DE, \
            io); \
  _F &= Sflag | Zflag | Cflag; \
  _F |= (_A + io) & Xflags; \
  _HL++; \
  _DE++; \
  _BC--; \
  if(_BC) _F |= Vflag; \
}

#define LDIR \
{ \
  LDI; \
  if(_BC) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define NEG \
{ \
tULong value = _A; \
  _A = 0; \
  SUB(value); \
}

#define OUTD \
{ \
tULong io = read_mem(NativeCPC, \
                     _HL); \
  _B--; \
  z80_OUT_handler(NativeCPC, \
                  Z80->Regs.BC, \
                  io); \
  _HL--; \
  _F = Z80->SZ[_B]; \
  if (io & Sflag) _F |= Nflag; \
  if ((((_C - 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
  if ((drep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
     _F |= Pflag; \
}

#define OTDR \
{ \
  OUTD; \
  if(_B) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define OUTI \
{ \
tULong io = read_mem(NativeCPC, \
                     _HL); \
  _B--; \
  z80_OUT_handler(NativeCPC, \
                  Z80->Regs.BC, \
                  io); \
  _HL++; \
  _F = Z80->SZ[_B]; \
  if (io & Sflag) _F |= Nflag; \
  if ((((_C + 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
  if ((irep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
     _F |= Pflag; \
}

#define OTIR \
{ \
  OUTI; \
  if(_B) \
  { \
    Z80->iCycleCount += cc_ex[bOpCode]; \
    _PC -= 2; \
  } \
}

#define RLD \
{ \
tULong n = read_mem(NativeCPC, \
                    _HL); \
  write_mem(NativeCPC, \
            _HL, \
            (n << 4) | (_A & 0x0f)); \
  _A = (_A & 0xf0) | (n >> 4); \
  _F = (_F & Cflag) | Z80->SZP[_A]; \
}

#define RRD \
{ \
tULong n = read_mem(NativeCPC, \
                    _HL); \
  write_mem(NativeCPC, \
            _HL, \
            (n >> 4) | (_A << 4)); \
  _A = (_A & 0xf0) | (n & 0x0f); \
  _F = (_F & Cflag) | Z80->SZP[_A]; \
}

#define SBC16(reg) \
{ \
tULong res = _HLdword - Z80->Regs.reg.d - (_F & Cflag); \
  _F = (((_HLdword ^ res ^ Z80->Regs.reg.d) >> 8) & Hflag) | Nflag | \
       ((res >> 16) & Cflag) | \
       ((res >> 8) & (Sflag | Xflags)) | \
       ((res & 0xffff) ? 0 : Zflag) | \
       (((Z80->Regs.reg.d ^ _HLdword) & (_HLdword ^ res) &0x8000) >> 13); \
  _HL = res; \
}


#define Z80_INT_HANDLER \
{ \
  if (_IFF1) /* process interrupts? */ \
  { \
    _R++; \
    _IFF1 = _IFF2 = 0; /* clear interrupt flip-flops */ \
    Z80->Regs.int_pending = 0; \
    NativeCPC->GateArray->sl_count &= 0x1f; /* clear bit 5 of GA scanline counter */ \
    \
    if (_HALT) /* HALT instruction active? */ \
    { \
      _HALT = 0; /* exit HALT 'loop' */ \
      _PC++; /* correct PC */ \
      bOpCode=0;\
    } \
    \
    if (_IM < 2) /* interrupt mode 0 or 1? (IM0 = IM1 on the CPC) */ \
    { \
      Z80->iCycleCount = 20; \
      if (Z80->iWSAdjust) \
      { \
        Z80->iCycleCount -= 4; \
      } \
      RST(0x0038); \
    } \
    else /* interrupt mode 2 */ \
    { \
      tRegister addr; \
      Z80->iCycleCount = 28; /* was 76 */ \
      if (Z80->iWSAdjust) \
      { \
        Z80->iCycleCount -= 4; \
      } \
      write_mem(NativeCPC, \
                --_SP, \
                Z80->Regs.PC.b.h); /* store high byte of current PC */ \
      write_mem(NativeCPC, \
                --_SP, \
                Z80->Regs.PC.b.l); /* store low byte of current PC */ \
      addr.b.l = 0xff; /* assemble pointer */ \
      addr.b.h = _I; \
      Z80->Regs.PC.b.l = read_mem(NativeCPC, \
                                  addr.w.l); /* retrieve low byte of vector */ \
      Z80->Regs.PC.b.h = read_mem(NativeCPC, \
                                  addr.w.l+1); /* retrieve high byte of vector */ \
    } \
    \
    Z80_WAIT_STATES \
  }\
}

//
// CAUTION : addr MUST be 16-bits
//
static inline tUChar read_mem(tNativeCPC* NativeCPC,
                       tULong addr)
{
#if defined(_DEBUG) && defined(_TRACE)
  if ((addr>>14) >= MEMBANK_NUMBER)
  {
    NativeCPC->TraceInstruction = 1;
    NativeCPC->TraceDisplay = 1;
    NativeCPC->TraceStop = 1;
    NativeCPC->TraceBreakpoint = TRACE_BP_read_mem;
    return 0; // Do not perform hazardous operation
  }
#endif

  PROFILE_ADD_NATIVE(PROFILE_read_mem);

  //Original
  //return (*(NativeCPC->membank_read[addr >> 14] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
  return (*(NativeCPC->membank_read[(addr >> 14) & 3] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
}


//
// CAUTION : addr MUST be 16-bits
//
static inline tChar signed_read_mem(tNativeCPC* NativeCPC,
                             tULong addr)
{
#if defined(_DEBUG) && defined(_TRACE)
  if ((addr>>14) >= MEMBANK_NUMBER)
  {
    NativeCPC->TraceInstruction = 1;
    NativeCPC->TraceDisplay = 1;
    NativeCPC->TraceStop = 1;
    NativeCPC->TraceBreakpoint = TRACE_BP_signed_read_mem;
    return 0; // Do not perform hazardous operation
  }
#endif

  PROFILE_ADD_NATIVE(PROFILE_read_mem);

  //Original
  //return (*(NativeCPC->membank_read[addr >> 14] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
  return (*(NativeCPC->membank_read[(addr >> 14) & 3] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank

}


//
// CAUTION : addr MUST be 16-bits
//
static inline tVoid write_mem(tNativeCPC* NativeCPC,
                       tULong addr,
                       tULong val)
{
#if defined(_DEBUG) && defined(_TRACE)
  if ((addr>>14) >= MEMBANK_NUMBER)
  {
    NativeCPC->TraceInstruction = 1;
    NativeCPC->TraceDisplay = 1;
    NativeCPC->TraceStop = 1;
    NativeCPC->TraceBreakpoint = TRACE_BP_write_mem;
    return; // Do not perform hazardous operation
  }
#endif

  PROFILE_ADD_NATIVE(PROFILE_write_mem);

  //Original
  //*(NativeCPC->membank_write[addr >> 14] + (addr & 0x3fff)) = (tUChar)val; // writes a byte to a 16KB memory bank
  *(NativeCPC->membank_write[(addr >> 14) & 3] + (addr & 0x3fff)) = (tUChar)val; // writes a byte to a 16KB memory bank
}


#define PSG_WRITE \
{ \
	tULong control; \
  PROFILE_ADD_NATIVE(PROFILE_PSG_WRITE); \
  \
  control = PSG->control & 0xc0; /* isolate PSG control bits */ \
  if (control == 0xc0) /* latch address? */ \
  { \
    PSG->reg_select = psg_data; /* select new PSG register */ \
  } \
  else if (control == 0x80) /* write? */ \
  { \
    if (PSG->reg_select < 16) /* valid register? */ \
    { \
      audio_set_AY_Register(PSG, \
                            PSG->reg_select, \
                            psg_data); \
    } \
  } \
}


#define LOAD_RESULT_WITH_STATUS \
{ \
  FDC->result[RES_ST0] |= 0x40; /* AT */ \
  FDC->result[RES_ST1] |= 0x80; /* End of Cylinder */ \
  \
  if (FDC->command[CMD_CODE] != 0x42) /* continue only if not a read track command */ \
  { \
    if ((FDC->result[RES_ST1] & 0x7f) || (FDC->result[RES_ST2] & 0x7f)) /* any 'error bits' set? */ \
    { \
      FDC->result[RES_ST1] &= 0x7f; /* mask out End of Cylinder */ \
      \
      if ((FDC->result[RES_ST1] & 0x20) || (FDC->result[RES_ST2] & 0x20)) /* DE and/or DD? */ \
      { \
        FDC->result[RES_ST2] &= 0xbf; /* mask out Control Mark */ \
      } \
      else if (FDC->result[RES_ST2] & 0x40) /* Control Mark? */ \
      { \
        FDC->result[RES_ST0] &= 0x3f; /* mask out AT */ \
        /*FDC->result[RES_ST1] &= 0x7f;*/ /* mask out End of Cylinder */ \
      } \
    } \
  } \
}


#define LOAD_RESULT_WITH_CHRN \
{ \
   FDC->result[RES_C] = FDC->command[CMD_C]; /* load result with current CHRN values */ \
   FDC->result[RES_H] = FDC->command[CMD_H]; \
   FDC->result[RES_R] = FDC->command[CMD_R]; \
   FDC->result[RES_N] = FDC->command[CMD_N]; \
}



/***********************************************************************
 *
 *  Entry Points
 *
 ***********************************************************************/

#ifdef _TRACE
#  define SHOWTRACE(p) \
{ \
tUChar argsTrace[4]; \
  if (NativeCPC->TraceDisplay) \
  { \
    NativeCPC->TraceFollowUp = (tULong)p; \
    *((tULong*)(&(argsTrace[0]))) = EndianSwap32(p); \
    NativeCPC->call68KFuncP(NativeCPC->emulStateP, \
                            NativeCPC->TraceAlertPtr, \
                            &argsTrace, \
                            (sizeof(argsTrace)/sizeof(argsTrace[0]))); \
  } \
}
#else /* _TRACE */
#  define SHOWTRACE(p)
#endif /* _TRACE */

#ifdef __PALMOS__
tULong PNOMain(const tVoid* emulStateP,
               tVoid* userData68KP,
               Call68KFuncType* call68KFuncP)
#else
tULong Engine_CPCExecute(tNativeCPC* NativeCPC)
#endif
/***********************************************************************
 *
 *  PNOMain
 *
 ***********************************************************************/
{
#ifdef __PALMOS__
tNativeCPC* NativeCPC = (tNativeCPC*)userData68KP;
#endif
tZ80* Z80 = NativeCPC->Z80;
tPSG* PSG = NativeCPC->PSG;
tFDC* FDC = NativeCPC->FDC;
tVDU* VDU = NativeCPC->VDU;
#ifdef _TESTU
tUShort Result;
#endif /* _TESTU */
tUChar bOpCode;

#ifdef __PALMOS__
  NOT_USED(emulStateP);
  NOT_USED(call68KFuncP);
#endif

  // First launch
  if (NativeCPC->FirstInitToPerform)
  {
#ifdef _TESTU
    Result = PerformTestU(NativeCPC);
    if (Result != errNone)
      return Result;
#endif /* _TESTU */
    fdc_init(NativeCPC);
    video_init(NativeCPC);
    audio_init(NativeCPC);
    NativeCPC->FirstInitToPerform = 0;
  }

  // Initialisation after Restoration
  if (NativeCPC->RestorationPerformed)
  {
    fdc_init(NativeCPC);
    video_init(NativeCPC);
    ga_memory_manager(NativeCPC);

    AUDIO_SET_CASE_ENV(PSG->RegisterAY.UChar.EnvType);

  	NativeCPC->RestorationPerformed = 0;
  }
  // CPU LOOP
  do
  {
    PROFILE_ADD_NATIVE(PROFILE_CPCExecute_Main_Loop);
    SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 0, 0));

    //if (_PC==0)
    //  printf("!!Reset!!\n");

    if (bOpCode!=halt)
    {
    bOpCode = read_mem(NativeCPC,
                       _PC++);

    //printf("%4x, %2x\n",_PC, bOpCode);

#ifdef _PROFILE
    if (NativeCPC->profileStates==1 || NativeCPC->profileStates==3)
    {
      printf("%2x,",bOpCode);
      NativeCPC->profileStates=3;
    }

#endif
    Z80->iCycleCount = cc_op[bOpCode];
    //_R++;

    SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 1, bOpCode));

    switch (bOpCode)
    {
      case adc_a:       ADC(_A); break;
      case adc_b:       ADC(_B); break;
      case adc_byte:    ADC(read_mem(NativeCPC, _PC++)); break;
      case adc_c:       ADC(_C); break;
      case adc_d:       ADC(_D); break;
      case adc_e:       ADC(_E); break;
      case adc_h:       ADC(_H); break;
      case adc_l:       ADC(_L); break;
      case adc_mhl:     ADC(read_mem(NativeCPC, _HL)); break;
      case add_a:       ADD(_A); break;
      case add_b:       ADD(_B); break;
      case add_byte:    ADD(read_mem(NativeCPC, _PC++)); break;
      case add_c:       ADD(_C); break;
      case add_d:       ADD(_D); break;
      case add_e:       ADD(_E); break;
      case add_h:       ADD(_H); break;
      case add_hl_bc:   ADD16(HL, BC); break;
      case add_hl_de:   ADD16(HL, DE); break;
      case add_hl_hl:   ADD16(HL, HL); break;
      case add_hl_sp:   ADD16(HL, SP); break;
      case add_l:       ADD(_L); break;
      case add_mhl:     ADD(read_mem(NativeCPC, _HL)); break;
      case and_a:       AND(_A); break;
      case and_b:       AND(_B); break;
      case and_byte:    AND(read_mem(NativeCPC, _PC++)); break;
      case and_c:       AND(_C); break;
      case and_d:       AND(_D); break;
      case and_e:       AND(_E); break;
      case and_h:       AND(_H); break;
      case and_l:       AND(_L); break;
      case and_mhl:     AND(read_mem(NativeCPC, _HL)); break;
      case call:        CALL; break;
      case call_c:      if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_m:      if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nc:     if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nz:     if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_p:      if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_pe:     if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_po:     if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_z:      if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & Cflag) << 4) | (_A & Xflags)) ^ Cflag; break;
      case cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); break;
      case cp_a:        CP(_A); break;
      case cp_b:        CP(_B); break;
      case cp_byte:     CP(read_mem(NativeCPC, _PC++)); break;
      case cp_c:        CP(_C); break;
      case cp_d:        CP(_D); break;
      case cp_e:        CP(_E); break;
      case cp_h:        CP(_H); break;
      case cp_l:        CP(_L); break;
      case cp_mhl:      CP(read_mem(NativeCPC, _HL)); break;
      case daa:         DAA; break;
      case dec_a:       DEC(_A); break;
      case dec_b:       DEC(_B); break;
      case dec_bc:      _BC--; Z80->iWSAdjust++; break;
      case dec_c:       DEC(_C); break;
      case dec_d:       DEC(_D); break;
      case dec_de:      _DE--; Z80->iWSAdjust++; break;
      case dec_e:       DEC(_E); break;
      case dec_h:       DEC(_H); break;
      case dec_hl:      _HL--; Z80->iWSAdjust++; break;
      case dec_l:       DEC(_L); break;
      case dec_mhl:     { tULong b = read_mem(NativeCPC, _HL); DEC(b); write_mem(NativeCPC, _HL, b); } break;
      case dec_sp:      _SP--; Z80->iWSAdjust++; break;
      case di:          _IFF1 = _IFF2 = 0; Z80->Regs.EI_issued = 0; break;
      case djnz:        if (--_B) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } break;
      case ei:          Z80->Regs.EI_issued = 2; break;
      case exx:         EXX; break;
      case ex_af_af:    EX(Z80->Regs.AF, Z80->Regs.AFx); break;
      case ex_de_hl:    EX(Z80->Regs.DE, Z80->Regs.HL); break;
      case ex_msp_hl:   EX_SP(HL); Z80->iWSAdjust++; break;
      case halt:        _HALT = 1; _PC--; break;
      case ina:         { Z80_WAIT_STATES Z80->iCycleCount = Ia_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; _A = z80_IN_handler(NativeCPC, p); } break;
      case inc_a:       INC(_A); break;
      case inc_b:       INC(_B); break;
      case inc_bc:      _BC++; Z80->iWSAdjust++; break;
      case inc_c:       INC(_C); break;
      case inc_d:       INC(_D); break;
      case inc_de:      _DE++; Z80->iWSAdjust++; break;
      case inc_e:       INC(_E); break;
      case inc_h:       INC(_H); break;
      case inc_hl:      _HL++; Z80->iWSAdjust++; break;
      case inc_l:       INC(_L); break;
      case inc_mhl:     { tULong b = read_mem(NativeCPC, _HL); INC(b); write_mem(NativeCPC, _HL, b); } break;
      case inc_sp:      _SP++; Z80->iWSAdjust++; break;
      case jp:          JP; break;
      case jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; break;
      case jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; break;
      case jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; break;
      case jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; break;
      case jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; break;
      case jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; break;
      case jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; break;
      case jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; break;
      case jr:          JR; break;
      case jr_c:        if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nc:       if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nz:       if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_z:        if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case ld_a_a:      break;
      case ld_a_b:      _A = _B; break;
      case ld_a_byte:   _A = read_mem(NativeCPC, _PC++); break;
      case ld_a_c:      _A = _C; break;
      case ld_a_d:      _A = _D; break;
      case ld_a_e:      _A = _E; break;
      case ld_a_h:      _A = _H; break;
      case ld_a_l:      _A = _L; break;
      case ld_a_mbc:    _A = read_mem(NativeCPC, _BC); break;
      case ld_a_mde:    _A = read_mem(NativeCPC, _DE); break;
      case ld_a_mhl:    _A = read_mem(NativeCPC, _HL); break;
      case ld_a_mword:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); _A = read_mem(NativeCPC, addr.w.l); } break;
      case ld_bc_word:  { _C = read_mem(NativeCPC, _PC++); _B = read_mem(NativeCPC, _PC++); } break;
      case ld_b_a:      _B = _A; break;
      case ld_b_b:      break;
      case ld_b_byte:   _B = read_mem(NativeCPC, _PC++); break;
      case ld_b_c:      _B = _C; break;
      case ld_b_d:      _B = _D; break;
      case ld_b_e:      _B = _E; break;
      case ld_b_h:      _B = _H; break;
      case ld_b_l:      _B = _L; break;
      case ld_b_mhl:    _B = read_mem(NativeCPC, _HL); break;
      case ld_c_a:      _C = _A; break;
      case ld_c_b:      _C = _B; break;
      case ld_c_byte:   _C = read_mem(NativeCPC, _PC++); break;
      case ld_c_c:      break;
      case ld_c_d:      _C = _D; break;
      case ld_c_e:      _C = _E; break;
      case ld_c_h:      _C = _H; break;
      case ld_c_l:      _C = _L; break;
      case ld_c_mhl:    _C = read_mem(NativeCPC, _HL); break;
      case ld_de_word:  { _E = read_mem(NativeCPC, _PC++); _D = read_mem(NativeCPC, _PC++); } break;
      case ld_d_a:      _D = _A; break;
      case ld_d_b:      _D = _B; break;
      case ld_d_byte:   _D = read_mem(NativeCPC, _PC++); break;
      case ld_d_c:      _D = _C; break;
      case ld_d_d:      break;
      case ld_d_e:      _D = _E; break;
      case ld_d_h:      _D = _H; break;
      case ld_d_l:      _D = _L; break;
      case ld_d_mhl:    _D = read_mem(NativeCPC, _HL); break;
      case ld_e_a:      _E = _A; break;
      case ld_e_b:      _E = _B; break;
      case ld_e_byte:   _E = read_mem(NativeCPC, _PC++); break;
      case ld_e_c:      _E = _C; break;
      case ld_e_d:      _E = _D; break;
      case ld_e_e:      break;
      case ld_e_h:      _E = _H; break;
      case ld_e_l:      _E = _L; break;
      case ld_e_mhl:    _E = read_mem(NativeCPC, _HL); break;
      case ld_hl_mword: LD16_MEM(HL); break;
      case ld_hl_word:  { _L = read_mem(NativeCPC, _PC++); _H = read_mem(NativeCPC, _PC++); } break;
      case ld_h_a:      _H = _A; break;
      case ld_h_b:      _H = _B; break;
      case ld_h_byte:   _H = read_mem(NativeCPC, _PC++); break;
      case ld_h_c:      _H = _C; break;
      case ld_h_d:      _H = _D; break;
      case ld_h_e:      _H = _E; break;
      case ld_h_h:      break;
      case ld_h_l:      _H = _L; break;
      case ld_h_mhl:    _H = read_mem(NativeCPC, _HL); break;
      case ld_l_a:      _L = _A; break;
      case ld_l_b:      _L = _B; break;
      case ld_l_byte:   _L = read_mem(NativeCPC, _PC++); break;
      case ld_l_c:      _L = _C; break;
      case ld_l_d:      _L = _D; break;
      case ld_l_e:      _L = _E; break;
      case ld_l_h:      _L = _H; break;
      case ld_l_l:      break;
      case ld_l_mhl:    _L = read_mem(NativeCPC, _HL); break;
      case ld_mbc_a:    write_mem(NativeCPC, _BC, _A); break;
      case ld_mde_a:    write_mem(NativeCPC, _DE, _A); break;
      case ld_mhl_a:    write_mem(NativeCPC, _HL, _A); break;
      case ld_mhl_b:    write_mem(NativeCPC, _HL, _B); break;
      case ld_mhl_byte: { tULong b = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, _HL, b); } break;
      case ld_mhl_c:    write_mem(NativeCPC, _HL, _C); break;
      case ld_mhl_d:    write_mem(NativeCPC, _HL, _D); break;
      case ld_mhl_e:    write_mem(NativeCPC, _HL, _E); break;
      case ld_mhl_h:    write_mem(NativeCPC, _HL, _H); break;
      case ld_mhl_l:    write_mem(NativeCPC, _HL, _L); break;
      case ld_mword_a:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, addr.w.l, _A); } break;
      case ld_mword_hl: LDMEM_16(HL); break;
      case ld_pc_hl:    _PC = _HL; break;
      case ld_sp_hl:    _SP = _HL; Z80->iWSAdjust++; break;
      case ld_sp_word:  { Z80->Regs.SP.b.l = read_mem(NativeCPC, _PC++); Z80->Regs.SP.b.h = read_mem(NativeCPC, _PC++); } break;
      case nop:         break;
      case or_a:        OR(_A); break;
      case or_b:        OR(_B); break;
      case or_byte:     OR(read_mem(NativeCPC, _PC++)); break;
      case or_c:        OR(_C); break;
      case or_d:        OR(_D); break;
      case or_e:        OR(_E); break;
      case or_h:        OR(_H); break;
      case or_l:        OR(_L); break;
      case or_mhl:      OR(read_mem(NativeCPC, _HL)); break;
      case outa:        { Z80_WAIT_STATES Z80->iCycleCount = Oa_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; z80_OUT_handler(NativeCPC, p, _A); } break;
      case pfx_cb:      z80_pfx_cb(NativeCPC); break;
      case pfx_dd:      z80_pfx_dd(NativeCPC); break;
      case pfx_ed:      z80_pfx_ed(NativeCPC); break;
      case pfx_fd:      z80_pfx_fd(NativeCPC); break;
      case pop_af:      POP(AF); break;
      case pop_bc:      POP(BC); break;
      case pop_de:      POP(DE); break;
      case pop_hl:      POP(HL); break;
      case push_af:     PUSH(AF); break;
      case push_bc:     PUSH(BC); break;
      case push_de:     PUSH(DE); break;
      case push_hl:     PUSH(HL); break;
      case ret:         RET; break;
      case ret_c:       if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_m:       if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_nc:      if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_nz:      if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_p:       if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_pe:      if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_po:      if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case ret_z:       if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
      case rla:         RLA; break;
      case rlca:        RLCA; break;
      case rra:         RRA; break;
      case rrca:        RRCA; break;
      case rst00:       break; //RST(0x0000); break; // Some games fall into this ? Fixed by ignoring it
      case rst08:       RST(0x0008); break;
      case rst10:       RST(0x0010); break;
      case rst18:       RST(0x0018); break;
      case rst20:       RST(0x0020); break;
      case rst28:       RST(0x0028); break;
      case rst30:       RST(0x0030); break;
      case rst38:       RST(0x0038); break;
      case sbc_a:       SBC(_A); break;
      case sbc_b:       SBC(_B); break;
      case sbc_byte:    SBC(read_mem(NativeCPC, _PC++)); break;
      case sbc_c:       SBC(_C); break;
      case sbc_d:       SBC(_D); break;
      case sbc_e:       SBC(_E); break;
      case sbc_h:       SBC(_H); break;
      case sbc_l:       SBC(_L); break;
      case sbc_mhl:     SBC(read_mem(NativeCPC, _HL)); break;
      case scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); break;
      case sub_a:       SUB(_A); break;
      case sub_b:       SUB(_B); break;
      case sub_byte:    SUB(read_mem(NativeCPC, _PC++)); break;
      case sub_c:       SUB(_C); break;
      case sub_d:       SUB(_D); break;
      case sub_e:       SUB(_E); break;
      case sub_h:       SUB(_H); break;
      case sub_l:       SUB(_L); break;
      case sub_mhl:     SUB(read_mem(NativeCPC, _HL)); break;
      case xor_a:       XOR(_A); break;
      case xor_b:       XOR(_B); break;
      case xor_byte:    XOR(read_mem(NativeCPC, _PC++)); break;
      case xor_c:       XOR(_C); break;
      case xor_d:       XOR(_D); break;
      case xor_e:       XOR(_E); break;
      case xor_h:       XOR(_H); break;
      case xor_l:       XOR(_L); break;
      case xor_mhl:     XOR(read_mem(NativeCPC, _HL)); break;
    }
  }

  _R++;

#if defined(_DEBUG) && defined(_TRACE)
    // Check registers coherence
    if ( (_AFdword > 0xffff) ||
         (_BCdword > 0xffff) ||
         (_DEdword > 0xffff) ||
         (_HLdword > 0xffff) ||
         (_IXdword > 0xffff) ||
         (_IYdword > 0xffff) ||
         (_PCdword > 0xffff) ||
         (_SPdword > 0xffff) )
    {
      NativeCPC->TraceInstruction = 1;
      NativeCPC->TraceDisplay = 1;
      NativeCPC->TraceStop = 1;
      NativeCPC->TraceBreakpoint = TRACE_BP_PNOMain_registers;
    }
#endif /* _DEBUG && _TRACE */

    SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 2, bOpCode));

    Z80_WAIT_STATES

    SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 3, bOpCode));

    if (Z80->Regs.EI_issued) // EI 'delay' in effect?
    {
      if (--Z80->Regs.EI_issued == 0)
      {
        _IFF1 = _IFF2 = Pflag; // set interrupt flip-flops
        if (Z80->Regs.int_pending)
        {
          Z80_INT_HANDLER
        }
      }
    }
    else if (Z80->Regs.int_pending) // any interrupts pending?
    {
      Z80_INT_HANDLER
    }

    Z80->iWSAdjust = 0;

    SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 4, bOpCode));

#ifdef _TRACE
    if (_PCdword == Z80->Regs.breakpoint)
    {
      SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 5, bOpCode));

      return EC_BREAKPOINT;
    }
    else
#endif /* _TRACE */

    //
    // Exit conditions
    //
    if (VDU->frame_completed) // video emulation finished building frame?
    {
      SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 6, bOpCode));

      VDU->frame_completed = 0;
      VDU->scr_base = (tULong*)NativeCPC->BmpOffScreenBits; // reset to surface start

      NativeCPC->drive_led = FDC->led;  // Copy to speed up main loop

      return EC_FRAME_COMPLETE; // exit emulation loop
    }
    else if (PSG->buffer_full) // sound emulation finished filling a buffer?
    {
      SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 7, bOpCode));

      PSG->buffer_full = 0;

      return EC_SOUND_BUFFER; // exit emulation loop
    }
    else if (NativeCPC->cycle_count <= 0) // emulation loop ran for one frame?
    {
      SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 8, bOpCode));

      NativeCPC->cycle_count += CYCLE_COUNT_INIT;

      return EC_CYCLE_COUNT; // exit emulation loop
    }

  } //end do CPU

#ifdef _TRACE
  while (!NativeCPC->TraceInstruction);

  SHOWTRACE(TRACE_DATA(TRACE_FN_CPCExecute_Main, 9, bOpCode));

  return EC_END_INSTRUCTION;
#else /* _TRACE */
  while (1);

#endif /* _TRACE */
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_cb(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_cb
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tUChar bOpCode;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_cb);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_cb, 0, 0));

  bOpCode = read_mem(NativeCPC, _PC++);
  Z80->iCycleCount += cc_cb[bOpCode];
  _R++;

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_cb, 1, bOpCode));

  switch (bOpCode)
  {
    case bit0_a:      BIT(0, _A); break;
    case bit0_b:      BIT(0, _B); break;
    case bit0_c:      BIT(0, _C); break;
    case bit0_d:      BIT(0, _D); break;
    case bit0_e:      BIT(0, _E); break;
    case bit0_h:      BIT(0, _H); break;
    case bit0_l:      BIT(0, _L); break;
    case bit0_mhl:    BIT(0, read_mem(NativeCPC, _HL)); break;
    case bit1_a:      BIT(1, _A); break;
    case bit1_b:      BIT(1, _B); break;
    case bit1_c:      BIT(1, _C); break;
    case bit1_d:      BIT(1, _D); break;
    case bit1_e:      BIT(1, _E); break;
    case bit1_h:      BIT(1, _H); break;
    case bit1_l:      BIT(1, _L); break;
    case bit1_mhl:    BIT(1, read_mem(NativeCPC, _HL)); break;
    case bit2_a:      BIT(2, _A); break;
    case bit2_b:      BIT(2, _B); break;
    case bit2_c:      BIT(2, _C); break;
    case bit2_d:      BIT(2, _D); break;
    case bit2_e:      BIT(2, _E); break;
    case bit2_h:      BIT(2, _H); break;
    case bit2_l:      BIT(2, _L); break;
    case bit2_mhl:    BIT(2, read_mem(NativeCPC, _HL)); break;
    case bit3_a:      BIT(3, _A); break;
    case bit3_b:      BIT(3, _B); break;
    case bit3_c:      BIT(3, _C); break;
    case bit3_d:      BIT(3, _D); break;
    case bit3_e:      BIT(3, _E); break;
    case bit3_h:      BIT(3, _H); break;
    case bit3_l:      BIT(3, _L); break;
    case bit3_mhl:    BIT(3, read_mem(NativeCPC, _HL)); break;
    case bit4_a:      BIT(4, _A); break;
    case bit4_b:      BIT(4, _B); break;
    case bit4_c:      BIT(4, _C); break;
    case bit4_d:      BIT(4, _D); break;
    case bit4_e:      BIT(4, _E); break;
    case bit4_h:      BIT(4, _H); break;
    case bit4_l:      BIT(4, _L); break;
    case bit4_mhl:    BIT(4, read_mem(NativeCPC, _HL)); break;
    case bit5_a:      BIT(5, _A); break;
    case bit5_b:      BIT(5, _B); break;
    case bit5_c:      BIT(5, _C); break;
    case bit5_d:      BIT(5, _D); break;
    case bit5_e:      BIT(5, _E); break;
    case bit5_h:      BIT(5, _H); break;
    case bit5_l:      BIT(5, _L); break;
    case bit5_mhl:    BIT(5, read_mem(NativeCPC, _HL)); break;
    case bit6_a:      BIT(6, _A); break;
    case bit6_b:      BIT(6, _B); break;
    case bit6_c:      BIT(6, _C); break;
    case bit6_d:      BIT(6, _D); break;
    case bit6_e:      BIT(6, _E); break;
    case bit6_h:      BIT(6, _H); break;
    case bit6_l:      BIT(6, _L); break;
    case bit6_mhl:    BIT(6, read_mem(NativeCPC, _HL)); break;
    case bit7_a:      BIT(7, _A); break;
    case bit7_b:      BIT(7, _B); break;
    case bit7_c:      BIT(7, _C); break;
    case bit7_d:      BIT(7, _D); break;
    case bit7_e:      BIT(7, _E); break;
    case bit7_h:      BIT(7, _H); break;
    case bit7_l:      BIT(7, _L); break;
    case bit7_mhl:    BIT(7, read_mem(NativeCPC, _HL)); break;
    case res0_a:      _A = RES(0, _A); break;
    case res0_b:      _B = RES(0, _B); break;
    case res0_c:      _C = RES(0, _C); break;
    case res0_d:      _D = RES(0, _D); break;
    case res0_e:      _E = RES(0, _E); break;
    case res0_h:      _H = RES(0, _H); break;
    case res0_l:      _L = RES(0, _L); break;
    case res0_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(0, b)); } break;
    case res1_a:      _A = RES(1, _A); break;
    case res1_b:      _B = RES(1, _B); break;
    case res1_c:      _C = RES(1, _C); break;
    case res1_d:      _D = RES(1, _D); break;
    case res1_e:      _E = RES(1, _E); break;
    case res1_h:      _H = RES(1, _H); break;
    case res1_l:      _L = RES(1, _L); break;
    case res1_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(1, b)); } break;
    case res2_a:      _A = RES(2, _A); break;
    case res2_b:      _B = RES(2, _B); break;
    case res2_c:      _C = RES(2, _C); break;
    case res2_d:      _D = RES(2, _D); break;
    case res2_e:      _E = RES(2, _E); break;
    case res2_h:      _H = RES(2, _H); break;
    case res2_l:      _L = RES(2, _L); break;
    case res2_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(2, b)); } break;
    case res3_a:      _A = RES(3, _A); break;
    case res3_b:      _B = RES(3, _B); break;
    case res3_c:      _C = RES(3, _C); break;
    case res3_d:      _D = RES(3, _D); break;
    case res3_e:      _E = RES(3, _E); break;
    case res3_h:      _H = RES(3, _H); break;
    case res3_l:      _L = RES(3, _L); break;
    case res3_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(3, b)); } break;
    case res4_a:      _A = RES(4, _A); break;
    case res4_b:      _B = RES(4, _B); break;
    case res4_c:      _C = RES(4, _C); break;
    case res4_d:      _D = RES(4, _D); break;
    case res4_e:      _E = RES(4, _E); break;
    case res4_h:      _H = RES(4, _H); break;
    case res4_l:      _L = RES(4, _L); break;
    case res4_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(4, b)); } break;
    case res5_a:      _A = RES(5, _A); break;
    case res5_b:      _B = RES(5, _B); break;
    case res5_c:      _C = RES(5, _C); break;
    case res5_d:      _D = RES(5, _D); break;
    case res5_e:      _E = RES(5, _E); break;
    case res5_h:      _H = RES(5, _H); break;
    case res5_l:      _L = RES(5, _L); break;
    case res5_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(5, b)); } break;
    case res6_a:      _A = RES(6, _A); break;
    case res6_b:      _B = RES(6, _B); break;
    case res6_c:      _C = RES(6, _C); break;
    case res6_d:      _D = RES(6, _D); break;
    case res6_e:      _E = RES(6, _E); break;
    case res6_h:      _H = RES(6, _H); break;
    case res6_l:      _L = RES(6, _L); break;
    case res6_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(6, b)); } break;
    case res7_a:      _A = RES(7, _A); break;
    case res7_b:      _B = RES(7, _B); break;
    case res7_c:      _C = RES(7, _C); break;
    case res7_d:      _D = RES(7, _D); break;
    case res7_e:      _E = RES(7, _E); break;
    case res7_h:      _H = RES(7, _H); break;
    case res7_l:      _L = RES(7, _L); break;
    case res7_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RES(7, b)); } break;
    case rlc_a:       _A = RLC(Z80, _A); break;
    case rlc_b:       _B = RLC(Z80, _B); break;
    case rlc_c:       _C = RLC(Z80, _C); break;
    case rlc_d:       _D = RLC(Z80, _D); break;
    case rlc_e:       _E = RLC(Z80, _E); break;
    case rlc_h:       _H = RLC(Z80, _H); break;
    case rlc_l:       _L = RLC(Z80, _L); break;
    case rlc_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RLC(Z80, b)); } break;
    case rl_a:        _A = RL(Z80, _A); break;
    case rl_b:        _B = RL(Z80, _B); break;
    case rl_c:        _C = RL(Z80, _C); break;
    case rl_d:        _D = RL(Z80, _D); break;
    case rl_e:        _E = RL(Z80, _E); break;
    case rl_h:        _H = RL(Z80, _H); break;
    case rl_l:        _L = RL(Z80, _L); break;
    case rl_mhl:      { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RL(Z80, b)); } break;
    case rrc_a:       _A = RRC(Z80, _A); break;
    case rrc_b:       _B = RRC(Z80, _B); break;
    case rrc_c:       _C = RRC(Z80, _C); break;
    case rrc_d:       _D = RRC(Z80, _D); break;
    case rrc_e:       _E = RRC(Z80, _E); break;
    case rrc_h:       _H = RRC(Z80, _H); break;
    case rrc_l:       _L = RRC(Z80, _L); break;
    case rrc_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RRC(Z80, b)); } break;
    case rr_a:        _A = RR(Z80, _A); break;
    case rr_b:        _B = RR(Z80, _B); break;
    case rr_c:        _C = RR(Z80, _C); break;
    case rr_d:        _D = RR(Z80, _D); break;
    case rr_e:        _E = RR(Z80, _E); break;
    case rr_h:        _H = RR(Z80, _H); break;
    case rr_l:        _L = RR(Z80, _L); break;
    case rr_mhl:      { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, RR(Z80, b)); } break;
    case set0_a:      _A = SET(0, _A); break;
    case set0_b:      _B = SET(0, _B); break;
    case set0_c:      _C = SET(0, _C); break;
    case set0_d:      _D = SET(0, _D); break;
    case set0_e:      _E = SET(0, _E); break;
    case set0_h:      _H = SET(0, _H); break;
    case set0_l:      _L = SET(0, _L); break;
    case set0_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(0, b)); } break;
    case set1_a:      _A = SET(1, _A); break;
    case set1_b:      _B = SET(1, _B); break;
    case set1_c:      _C = SET(1, _C); break;
    case set1_d:      _D = SET(1, _D); break;
    case set1_e:      _E = SET(1, _E); break;
    case set1_h:      _H = SET(1, _H); break;
    case set1_l:      _L = SET(1, _L); break;
    case set1_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(1, b)); } break;
    case set2_a:      _A = SET(2, _A); break;
    case set2_b:      _B = SET(2, _B); break;
    case set2_c:      _C = SET(2, _C); break;
    case set2_d:      _D = SET(2, _D); break;
    case set2_e:      _E = SET(2, _E); break;
    case set2_h:      _H = SET(2, _H); break;
    case set2_l:      _L = SET(2, _L); break;
    case set2_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(2, b)); } break;
    case set3_a:      _A = SET(3, _A); break;
    case set3_b:      _B = SET(3, _B); break;
    case set3_c:      _C = SET(3, _C); break;
    case set3_d:      _D = SET(3, _D); break;
    case set3_e:      _E = SET(3, _E); break;
    case set3_h:      _H = SET(3, _H); break;
    case set3_l:      _L = SET(3, _L); break;
    case set3_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(3, b)); } break;
    case set4_a:      _A = SET(4, _A); break;
    case set4_b:      _B = SET(4, _B); break;
    case set4_c:      _C = SET(4, _C); break;
    case set4_d:      _D = SET(4, _D); break;
    case set4_e:      _E = SET(4, _E); break;
    case set4_h:      _H = SET(4, _H); break;
    case set4_l:      _L = SET(4, _L); break;
    case set4_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(4, b)); } break;
    case set5_a:      _A = SET(5, _A); break;
    case set5_b:      _B = SET(5, _B); break;
    case set5_c:      _C = SET(5, _C); break;
    case set5_d:      _D = SET(5, _D); break;
    case set5_e:      _E = SET(5, _E); break;
    case set5_h:      _H = SET(5, _H); break;
    case set5_l:      _L = SET(5, _L); break;
    case set5_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(5, b)); } break;
    case set6_a:      _A = SET(6, _A); break;
    case set6_b:      _B = SET(6, _B); break;
    case set6_c:      _C = SET(6, _C); break;
    case set6_d:      _D = SET(6, _D); break;
    case set6_e:      _E = SET(6, _E); break;
    case set6_h:      _H = SET(6, _H); break;
    case set6_l:      _L = SET(6, _L); break;
    case set6_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(6, b)); } break;
    case set7_a:      _A = SET(7, _A); break;
    case set7_b:      _B = SET(7, _B); break;
    case set7_c:      _C = SET(7, _C); break;
    case set7_d:      _D = SET(7, _D); break;
    case set7_e:      _E = SET(7, _E); break;
    case set7_h:      _H = SET(7, _H); break;
    case set7_l:      _L = SET(7, _L); break;
    case set7_mhl:    { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SET(7, b)); } break;
    case sla_a:       _A = SLA(Z80, _A); break;
    case sla_b:       _B = SLA(Z80, _B); break;
    case sla_c:       _C = SLA(Z80, _C); break;
    case sla_d:       _D = SLA(Z80, _D); break;
    case sla_e:       _E = SLA(Z80, _E); break;
    case sla_h:       _H = SLA(Z80, _H); break;
    case sla_l:       _L = SLA(Z80, _L); break;
    case sla_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SLA(Z80, b)); } break;
    case sll_a:       _A = SLL(Z80, _A); break;
    case sll_b:       _B = SLL(Z80, _B); break;
    case sll_c:       _C = SLL(Z80, _C); break;
    case sll_d:       _D = SLL(Z80, _D); break;
    case sll_e:       _E = SLL(Z80, _E); break;
    case sll_h:       _H = SLL(Z80, _H); break;
    case sll_l:       _L = SLL(Z80, _L); break;
    case sll_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SLL(Z80, b)); } break;
    case sra_a:       _A = SRA(Z80, _A); break;
    case sra_b:       _B = SRA(Z80, _B); break;
    case sra_c:       _C = SRA(Z80, _C); break;
    case sra_d:       _D = SRA(Z80, _D); break;
    case sra_e:       _E = SRA(Z80, _E); break;
    case sra_h:       _H = SRA(Z80, _H); break;
    case sra_l:       _L = SRA(Z80, _L); break;
    case sra_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SRA(Z80, b)); } break;
    case srl_a:       _A = SRL(Z80, _A); break;
    case srl_b:       _B = SRL(Z80, _B); break;
    case srl_c:       _C = SRL(Z80, _C); break;
    case srl_d:       _D = SRL(Z80, _D); break;
    case srl_e:       _E = SRL(Z80, _E); break;
    case srl_h:       _H = SRL(Z80, _H); break;
    case srl_l:       _L = SRL(Z80, _L); break;
    case srl_mhl:     { tULong b = read_mem(NativeCPC, _HL); write_mem(NativeCPC, _HL, SRL(Z80, b)); } break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_cb, 2, bOpCode));
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_dd(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_dd
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tPSG* PSG = NativeCPC->PSG;
tFDC* FDC = NativeCPC->FDC;
tUChar bOpCode;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_dd);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_dd, 0, 0));

  bOpCode = read_mem(NativeCPC, _PC++);
  Z80->iCycleCount += cc_xy[bOpCode];
  _R++;

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_dd, 1, bOpCode));

  switch (bOpCode)
  {
    case adc_a:       ADC(_A); break;
    case adc_b:       ADC(_B); break;
    case adc_byte:    ADC(read_mem(NativeCPC, _PC++)); break;
    case adc_c:       ADC(_C); break;
    case adc_d:       ADC(_D); break;
    case adc_e:       ADC(_E); break;
    case adc_h:       ADC(_IXh); break;
    case adc_l:       ADC(_IXl); break;
    case adc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); ADC(read_mem(NativeCPC, (_IX+o))); } break;
    case add_a:       ADD(_A); break;
    case add_b:       ADD(_B); break;
    case add_byte:    ADD(read_mem(NativeCPC, _PC++)); break;
    case add_c:       ADD(_C); break;
    case add_d:       ADD(_D); break;
    case add_e:       ADD(_E); break;
    case add_h:       ADD(_IXh); break;
    case add_hl_bc:   ADD16(IX, BC); break;
    case add_hl_de:   ADD16(IX, DE); break;
    case add_hl_hl:   ADD16(IX, IX); break;
    case add_hl_sp:   ADD16(IX, SP); break;
    case add_l:       ADD(_IXl); break;
    case add_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); ADD(read_mem(NativeCPC, (_IX+o))); } break;
    case and_a:       AND(_A); break;
    case and_b:       AND(_B); break;
    case and_byte:    AND(read_mem(NativeCPC, _PC++)); break;
    case and_c:       AND(_C); break;
    case and_d:       AND(_D); break;
    case and_e:       AND(_E); break;
    case and_h:       AND(_IXh); break;
    case and_l:       AND(_IXl); break;
    case and_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); AND(read_mem(NativeCPC, (_IX+o))); } break;
    case call:        CALL; break;
    case call_c:      if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_m:      if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_nc:     if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_nz:     if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_p:      if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_pe:     if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_po:     if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_z:      if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & Cflag) << 4) | (_A & Xflags)) ^ Cflag; break;
    case cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); break;
    case cp_a:        CP(_A); break;
    case cp_b:        CP(_B); break;
    case cp_byte:     CP(read_mem(NativeCPC, _PC++)); break;
    case cp_c:        CP(_C); break;
    case cp_d:        CP(_D); break;
    case cp_e:        CP(_E); break;
    case cp_h:        CP(_IXh); break;
    case cp_l:        CP(_IXl); break;
    case cp_mhl:      { tLong o = signed_read_mem(NativeCPC, _PC++); CP(read_mem(NativeCPC, (tUShort)(_IX+o))); } break;
    case daa:         DAA; break;
    case dec_a:       DEC(_A); break;
    case dec_b:       DEC(_B); break;
    case dec_bc:      _BC--; Z80->iWSAdjust++; break;
    case dec_c:       DEC(_C); break;
    case dec_d:       DEC(_D); break;
    case dec_de:      _DE--; Z80->iWSAdjust++; break;
    case dec_e:       DEC(_E); break;
    case dec_h:       DEC(_IXh); break;
    case dec_hl:      _IX--; Z80->iWSAdjust++; break;
    case dec_l:       DEC(_IXl); break;
    case dec_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, (_IX+o)); DEC(b); write_mem(NativeCPC, (_IX+o), b); } break;
    case dec_sp:      _SP--; Z80->iWSAdjust++; break;
    case di:          _IFF1 = _IFF2 = 0; Z80->Regs.EI_issued = 0; break;
    case djnz:        if (--_B) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } break;
    case ei:          Z80->Regs.EI_issued = 2; break;
    case exx:         EXX; break;
    case ex_af_af:    EX(Z80->Regs.AF, Z80->Regs.AFx); break;
    case ex_de_hl:    EX(Z80->Regs.DE, Z80->Regs.HL); break;
    case ex_msp_hl:   EX_SP(IX); Z80->iWSAdjust++; break;
    case halt:        _HALT = 1; _PC--; break;
    case ina:         { Z80_WAIT_STATES Z80->iCycleCount = Ia_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; _A = z80_IN_handler(NativeCPC, p); } break;
    case inc_a:       INC(_A); break;
    case inc_b:       INC(_B); break;
    case inc_bc:      _BC++; Z80->iWSAdjust++; break;
    case inc_c:       INC(_C); break;
    case inc_d:       INC(_D); break;
    case inc_de:      _DE++; Z80->iWSAdjust++; break;
    case inc_e:       INC(_E); break;
    case inc_h:       INC(_IXh); break;
    case inc_hl:      _IX++; Z80->iWSAdjust++; break;
    case inc_l:       INC(_IXl); break;
    case inc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, (_IX+o)); INC(b); write_mem(NativeCPC, (_IX+o), b); } break;
    case inc_sp:      _SP++; Z80->iWSAdjust++; break;
    case jp:          JP; break;
    case jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; break;
    case jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; break;
    case jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; break;
    case jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; break;
    case jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; break;
    case jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; break;
    case jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; break;
    case jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; break;
    case jr:          JR; break;
    case jr_c:        if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_nc:       if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_nz:       if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_z:        if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case ld_a_a:      break;
    case ld_a_b:      _A = _B; break;
    case ld_a_byte:   _A = read_mem(NativeCPC, _PC++); break;
    case ld_a_c:      _A = _C; break;
    case ld_a_d:      _A = _D; break;
    case ld_a_e:      _A = _E; break;
    case ld_a_h:      _A = _IXh; break;
    case ld_a_l:      _A = _IXl; break;
    case ld_a_mbc:    _A = read_mem(NativeCPC, _BC); break;
    case ld_a_mde:    _A = read_mem(NativeCPC, _DE); break;
    case ld_a_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _A = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_a_mword:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); _A = read_mem(NativeCPC, addr.w.l); } break;
    case ld_bc_word:  { _C = read_mem(NativeCPC, _PC++); _B = read_mem(NativeCPC, _PC++); } break;
    case ld_b_a:      _B = _A; break;
    case ld_b_b:      break;
    case ld_b_byte:   _B = read_mem(NativeCPC, _PC++); break;
    case ld_b_c:      _B = _C; break;
    case ld_b_d:      _B = _D; break;
    case ld_b_e:      _B = _E; break;
    case ld_b_h:      _B = _IXh; break;
    case ld_b_l:      _B = _IXl; break;
    case ld_b_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _B = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_c_a:      _C = _A; break;
    case ld_c_b:      _C = _B; break;
    case ld_c_byte:   _C = read_mem(NativeCPC, _PC++); break;
    case ld_c_c:      break;
    case ld_c_d:      _C = _D; break;
    case ld_c_e:      _C = _E; break;
    case ld_c_h:      _C = _IXh; break;
    case ld_c_l:      _C = _IXl; break;
    case ld_c_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _C = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_de_word:  { _E = read_mem(NativeCPC, _PC++); _D = read_mem(NativeCPC, _PC++); } break;
    case ld_d_a:      _D = _A; break;
    case ld_d_b:      _D = _B; break;
    case ld_d_byte:   _D = read_mem(NativeCPC, _PC++); break;
    case ld_d_c:      _D = _C; break;
    case ld_d_d:      break;
    case ld_d_e:      _D = _E; break;
    case ld_d_h:      _D = _IXh; break;
    case ld_d_l:      _D = _IXl; break;
    case ld_d_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _D = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_e_a:      _E = _A; break;
    case ld_e_b:      _E = _B; break;
    case ld_e_byte:   _E = read_mem(NativeCPC, _PC++); break;
    case ld_e_c:      _E = _C; break;
    case ld_e_d:      _E = _D; break;
    case ld_e_e:      break;
    case ld_e_h:      _E = _IXh; break;
    case ld_e_l:      _E = _IXl; break;
    case ld_e_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _E = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_hl_mword: LD16_MEM(IX); break;
    case ld_hl_word:  { _IXl = read_mem(NativeCPC, _PC++); _IXh = read_mem(NativeCPC, _PC++); } break;
    case ld_h_a:      _IXh = _A; break;
    case ld_h_b:      _IXh = _B; break;
    case ld_h_byte:   _IXh = read_mem(NativeCPC, _PC++); break;
    case ld_h_c:      _IXh = _C; break;
    case ld_h_d:      _IXh = _D; break;
    case ld_h_e:      _IXh = _E; break;
    case ld_h_h:      break;
    case ld_h_l:      _IXh = _IXl; break;
    case ld_h_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _H = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_l_a:      _IXl = _A; break;
    case ld_l_b:      _IXl = _B; break;
    case ld_l_byte:   _IXl = read_mem(NativeCPC, _PC++); break;
    case ld_l_c:      _IXl = _C; break;
    case ld_l_d:      _IXl = _D; break;
    case ld_l_e:      _IXl = _E; break;
    case ld_l_h:      _IXl = _IXh; break;
    case ld_l_l:      break;
    case ld_l_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _L = read_mem(NativeCPC, (_IX+o)); } break;
    case ld_mbc_a:    write_mem(NativeCPC, _BC, _A); break;
    case ld_mde_a:    write_mem(NativeCPC, _DE, _A); break;
    case ld_mhl_a:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _A); } break;
    case ld_mhl_b:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _B); } break;
    case ld_mhl_byte: { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), b); } break;
    case ld_mhl_c:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _C); } break;
    case ld_mhl_d:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _D); } break;
    case ld_mhl_e:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _E); } break;
    case ld_mhl_h:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _H); } break;
    case ld_mhl_l:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IX+o), _L); } break;
    case ld_mword_a:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, addr.w.l, _A); } break;
    case ld_mword_hl: LDMEM_16(IX); break;
    case ld_pc_hl:    _PC = _IX; break;
    case ld_sp_hl:    _SP = _IX; Z80->iWSAdjust++; break;
    case ld_sp_word:  Z80->Regs.SP.b.l = read_mem(NativeCPC, _PC++); Z80->Regs.SP.b.h = read_mem(NativeCPC, _PC++); break;
    case nop:         break;
    case or_a:        OR(_A); break;
    case or_b:        OR(_B); break;
    case or_byte:     OR(read_mem(NativeCPC, _PC++)); break;
    case or_c:        OR(_C); break;
    case or_d:        OR(_D); break;
    case or_e:        OR(_E); break;
    case or_h:        OR(_IXh); break;
    case or_l:        OR(_IXl); break;
    case or_mhl:      { tLong o = signed_read_mem(NativeCPC, _PC++); OR(read_mem(NativeCPC, (_IX+o))); } break;
    case outa:        { Z80_WAIT_STATES Z80->iCycleCount = Oa_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; z80_OUT_handler(NativeCPC, p, _A); } break;
    case pfx_cb:      z80_pfx_ddcb(NativeCPC); break;
    case pfx_dd:      z80_pfx_dd(NativeCPC); break;
    case pfx_ed:      z80_pfx_ed(NativeCPC); break;
    case pfx_fd:      z80_pfx_fd(NativeCPC); break;
    case pop_af:      POP(AF); break;
    case pop_bc:      POP(BC); break;
    case pop_de:      POP(DE); break;
    case pop_hl:      POP(IX); break;
    case push_af:     PUSH(AF); break;
    case push_bc:     PUSH(BC); break;
    case push_de:     PUSH(DE); break;
    case push_hl:     PUSH(IX); break;
    case ret:         RET; break;
    case ret_c:       if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_m:       if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_nc:      if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_nz:      if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_p:       if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_pe:      if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_po:      if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_z:       if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case rla:         RLA; break;
    case rlca:        RLCA; break;
    case rra:         RRA; break;
    case rrca:        RRCA; break;
    case rst00:       RST(0x0000); break;
    case rst08:       RST(0x0008); break;
    case rst10:       RST(0x0010); break;
    case rst18:       RST(0x0018); break;
    case rst20:       RST(0x0020); break;
    case rst28:       RST(0x0028); break;
    case rst30:       RST(0x0030); break;
    case rst38:       RST(0x0038); break;
    case sbc_a:       SBC(_A); break;
    case sbc_b:       SBC(_B); break;
    case sbc_byte:    SBC(read_mem(NativeCPC, _PC++)); break;
    case sbc_c:       SBC(_C); break;
    case sbc_d:       SBC(_D); break;
    case sbc_e:       SBC(_E); break;
    case sbc_h:       SBC(_IXh); break;
    case sbc_l:       SBC(_IXl); break;
    case sbc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); SBC(read_mem(NativeCPC, (_IX+o))); } break;
    case scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); break;
    case sub_a:       SUB(_A); break;
    case sub_b:       SUB(_B); break;
    case sub_byte:    SUB(read_mem(NativeCPC, _PC++)); break;
    case sub_c:       SUB(_C); break;
    case sub_d:       SUB(_D); break;
    case sub_e:       SUB(_E); break;
    case sub_h:       SUB(_IXh); break;
    case sub_l:       SUB(_IXl); break;
    case sub_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); SUB(read_mem(NativeCPC, (_IX+o))); } break;
    case xor_a:       XOR(_A); break;
    case xor_b:       XOR(_B); break;
    case xor_byte:    XOR(read_mem(NativeCPC, _PC++)); break;
    case xor_c:       XOR(_C); break;
    case xor_d:       XOR(_D); break;
    case xor_e:       XOR(_E); break;
    case xor_h:       XOR(_IXh); break;
    case xor_l:       XOR(_IXl); break;
    case xor_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); XOR(read_mem(NativeCPC, (_IX+o))); } break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_dd, 2, bOpCode));
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_ddcb(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_ddcb
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tULong addr;
tLong o;
tUChar bOpCode;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_ddcb);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ddcb, 0, 0));

  o = signed_read_mem(NativeCPC, _PC++); // offset
  addr = _IX + o;
  bOpCode = read_mem(NativeCPC, _PC++);
  Z80->iCycleCount += cc_xycb[bOpCode];

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ddcb, 1, bOpCode));

  switch (bOpCode)
  {
    case bit0_a:
    case bit0_b:
    case bit0_c:
    case bit0_d:
    case bit0_e:
    case bit0_h:
    case bit0_l:
    case bit0_mhl:    BIT_XY(0, read_mem(NativeCPC, addr)); break;
    case bit1_a:
    case bit1_b:
    case bit1_c:
    case bit1_d:
    case bit1_e:
    case bit1_h:
    case bit1_l:
    case bit1_mhl:    BIT_XY(1, read_mem(NativeCPC, addr)); break;
    case bit2_a:
    case bit2_b:
    case bit2_c:
    case bit2_d:
    case bit2_e:
    case bit2_h:
    case bit2_l:
    case bit2_mhl:    BIT_XY(2, read_mem(NativeCPC, addr)); break;
    case bit3_a:
    case bit3_b:
    case bit3_c:
    case bit3_d:
    case bit3_e:
    case bit3_h:
    case bit3_l:
    case bit3_mhl:    BIT_XY(3, read_mem(NativeCPC, addr)); break;
    case bit4_a:
    case bit4_b:
    case bit4_c:
    case bit4_d:
    case bit4_e:
    case bit4_h:
    case bit4_l:
    case bit4_mhl:    BIT_XY(4, read_mem(NativeCPC, addr)); break;
    case bit5_a:
    case bit5_b:
    case bit5_c:
    case bit5_d:
    case bit5_e:
    case bit5_h:
    case bit5_l:
    case bit5_mhl:    BIT_XY(5, read_mem(NativeCPC, addr)); break;
    case bit6_a:
    case bit6_b:
    case bit6_c:
    case bit6_d:
    case bit6_e:
    case bit6_h:
    case bit6_l:
    case bit6_mhl:    BIT_XY(6, read_mem(NativeCPC, addr)); break;
    case bit7_a:
    case bit7_b:
    case bit7_c:
    case bit7_d:
    case bit7_e:
    case bit7_h:
    case bit7_l:
    case bit7_mhl:    BIT_XY(7, read_mem(NativeCPC, addr)); break;
    case res0_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(0, _A)); break;
    case res0_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(0, _B)); break;
    case res0_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(0, _C)); break;
    case res0_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(0, _D)); break;
    case res0_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(0, _E)); break;
    case res0_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(0, _H)); break;
    case res0_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(0, _L)); break;
    case res0_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(0, b)); } break;
    case res1_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(1, _A)); break;
    case res1_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(1, _B)); break;
    case res1_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(1, _C)); break;
    case res1_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(1, _D)); break;
    case res1_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(1, _E)); break;
    case res1_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(1, _H)); break;
    case res1_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(1, _L)); break;
    case res1_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(1, b)); } break;
    case res2_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(2, _A)); break;
    case res2_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(2, _B)); break;
    case res2_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(2, _C)); break;
    case res2_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(2, _D)); break;
    case res2_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(2, _E)); break;
    case res2_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(2, _H)); break;
    case res2_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(2, _L)); break;
    case res2_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(2, b)); } break;
    case res3_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(3, _A)); break;
    case res3_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(3, _B)); break;
    case res3_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(3, _C)); break;
    case res3_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(3, _D)); break;
    case res3_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(3, _E)); break;
    case res3_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(3, _H)); break;
    case res3_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(3, _L)); break;
    case res3_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(3, b)); } break;
    case res4_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(4, _A)); break;
    case res4_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(4, _B)); break;
    case res4_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(4, _C)); break;
    case res4_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(4, _D)); break;
    case res4_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(4, _E)); break;
    case res4_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(4, _H)); break;
    case res4_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(4, _L)); break;
    case res4_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(4, b)); } break;
    case res5_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(5, _A)); break;
    case res5_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(5, _B)); break;
    case res5_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(5, _C)); break;
    case res5_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(5, _D)); break;
    case res5_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(5, _E)); break;
    case res5_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(5, _H)); break;
    case res5_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(5, _L)); break;
    case res5_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(5, b)); } break;
    case res6_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(6, _A)); break;
    case res6_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(6, _B)); break;
    case res6_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(6, _C)); break;
    case res6_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(6, _D)); break;
    case res6_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(6, _E)); break;
    case res6_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(6, _H)); break;
    case res6_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(6, _L)); break;
    case res6_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(6, b)); } break;
    case res7_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(7, _A)); break;
    case res7_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(7, _B)); break;
    case res7_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(7, _C)); break;
    case res7_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(7, _D)); break;
    case res7_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(7, _E)); break;
    case res7_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(7, _H)); break;
    case res7_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(7, _L)); break;
    case res7_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(7, b)); } break;
    case rlc_a:       _A = read_mem(NativeCPC, addr); _A = RLC(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rlc_b:       _B = read_mem(NativeCPC, addr); _B = RLC(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rlc_c:       _C = read_mem(NativeCPC, addr); _C = RLC(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rlc_d:       _D = read_mem(NativeCPC, addr); _D = RLC(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rlc_e:       _E = read_mem(NativeCPC, addr); _E = RLC(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rlc_h:       _H = read_mem(NativeCPC, addr); _H = RLC(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rlc_l:       _L = read_mem(NativeCPC, addr); _L = RLC(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rlc_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RLC(Z80, b)); } break;
    case rl_a:        _A = read_mem(NativeCPC, addr); _A = RL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rl_b:        _B = read_mem(NativeCPC, addr); _B = RL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rl_c:        _C = read_mem(NativeCPC, addr); _C = RL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rl_d:        _D = read_mem(NativeCPC, addr); _D = RL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rl_e:        _E = read_mem(NativeCPC, addr); _E = RL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rl_h:        _H = read_mem(NativeCPC, addr); _H = RL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rl_l:        _L = read_mem(NativeCPC, addr); _L = RL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rl_mhl:      { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RL(Z80, b)); } break;
    case rrc_a:       _A = read_mem(NativeCPC, addr); _A = RRC(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rrc_b:       _B = read_mem(NativeCPC, addr); _B = RRC(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rrc_c:       _C = read_mem(NativeCPC, addr); _C = RRC(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rrc_d:       _D = read_mem(NativeCPC, addr); _D = RRC(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rrc_e:       _E = read_mem(NativeCPC, addr); _E = RRC(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rrc_h:       _H = read_mem(NativeCPC, addr); _H = RRC(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rrc_l:       _L = read_mem(NativeCPC, addr); _L = RRC(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rrc_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RRC(Z80, b)); } break;
    case rr_a:        _A = read_mem(NativeCPC, addr); _A = RR(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rr_b:        _B = read_mem(NativeCPC, addr); _B = RR(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rr_c:        _C = read_mem(NativeCPC, addr); _C = RR(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rr_d:        _D = read_mem(NativeCPC, addr); _D = RR(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rr_e:        _E = read_mem(NativeCPC, addr); _E = RR(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rr_h:        _H = read_mem(NativeCPC, addr); _H = RR(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rr_l:        _L = read_mem(NativeCPC, addr); _L = RR(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rr_mhl:      { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RR(Z80, b)); } break;
    case set0_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(0, _A)); break;
    case set0_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(0, _B)); break;
    case set0_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(0, _C)); break;
    case set0_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(0, _D)); break;
    case set0_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(0, _E)); break;
    case set0_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(0, _H)); break;
    case set0_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(0, _L)); break;
    case set0_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(0, b)); } break;
    case set1_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(1, _A)); break;
    case set1_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(1, _B)); break;
    case set1_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(1, _C)); break;
    case set1_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(1, _D)); break;
    case set1_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(1, _E)); break;
    case set1_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(1, _H)); break;
    case set1_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(1, _L)); break;
    case set1_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(1, b)); } break;
    case set2_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(2, _A)); break;
    case set2_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(2, _B)); break;
    case set2_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(2, _C)); break;
    case set2_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(2, _D)); break;
    case set2_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(2, _E)); break;
    case set2_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(2, _H)); break;
    case set2_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(2, _L)); break;
    case set2_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(2, b)); } break;
    case set3_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(3, _A)); break;
    case set3_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(3, _B)); break;
    case set3_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(3, _C)); break;
    case set3_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(3, _D)); break;
    case set3_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(3, _E)); break;
    case set3_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(3, _H)); break;
    case set3_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(3, _L)); break;
    case set3_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(3, b)); } break;
    case set4_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(4, _A)); break;
    case set4_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(4, _B)); break;
    case set4_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(4, _C)); break;
    case set4_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(4, _D)); break;
    case set4_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(4, _E)); break;
    case set4_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(4, _H)); break;
    case set4_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(4, _L)); break;
    case set4_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(4, b)); } break;
    case set5_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(5, _A)); break;
    case set5_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(5, _B)); break;
    case set5_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(5, _C)); break;
    case set5_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(5, _D)); break;
    case set5_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(5, _E)); break;
    case set5_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(5, _H)); break;
    case set5_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(5, _L)); break;
    case set5_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(5, b)); } break;
    case set6_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(6, _A)); break;
    case set6_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(6, _B)); break;
    case set6_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(6, _C)); break;
    case set6_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(6, _D)); break;
    case set6_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(6, _E)); break;
    case set6_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(6, _H)); break;
    case set6_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(6, _L)); break;
    case set6_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(6, b)); } break;
    case set7_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(7, _A)); break;
    case set7_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(7, _B)); break;
    case set7_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(7, _C)); break;
    case set7_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(7, _D)); break;
    case set7_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(7, _E)); break;
    case set7_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(7, _H)); break;
    case set7_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(7, _L)); break;
    case set7_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(7, b)); } break;
    case sla_a:       _A = read_mem(NativeCPC, addr); _A = SLA(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sla_b:       _B = read_mem(NativeCPC, addr); _B = SLA(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sla_c:       _C = read_mem(NativeCPC, addr); _C = SLA(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sla_d:       _D = read_mem(NativeCPC, addr); _D = SLA(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sla_e:       _E = read_mem(NativeCPC, addr); _E = SLA(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sla_h:       _H = read_mem(NativeCPC, addr); _H = SLA(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sla_l:       _L = read_mem(NativeCPC, addr); _L = SLA(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sla_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SLA(Z80, b)); } break;
    case sll_a:       _A = read_mem(NativeCPC, addr); _A = SLL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sll_b:       _B = read_mem(NativeCPC, addr); _B = SLL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sll_c:       _C = read_mem(NativeCPC, addr); _C = SLL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sll_d:       _D = read_mem(NativeCPC, addr); _D = SLL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sll_e:       _E = read_mem(NativeCPC, addr); _E = SLL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sll_h:       _H = read_mem(NativeCPC, addr); _H = SLL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sll_l:       _L = read_mem(NativeCPC, addr); _L = SLL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sll_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SLL(Z80, b)); } break;
    case sra_a:       _A = read_mem(NativeCPC, addr); _A = SRA(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sra_b:       _B = read_mem(NativeCPC, addr); _B = SRA(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sra_c:       _C = read_mem(NativeCPC, addr); _C = SRA(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sra_d:       _D = read_mem(NativeCPC, addr); _D = SRA(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sra_e:       _E = read_mem(NativeCPC, addr); _E = SRA(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sra_h:       _H = read_mem(NativeCPC, addr); _H = SRA(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sra_l:       _L = read_mem(NativeCPC, addr); _L = SRA(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sra_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SRA(Z80, b)); } break;
    case srl_a:       _A = read_mem(NativeCPC, addr); _A = SRL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case srl_b:       _B = read_mem(NativeCPC, addr); _B = SRL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case srl_c:       _C = read_mem(NativeCPC, addr); _C = SRL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case srl_d:       _D = read_mem(NativeCPC, addr); _D = SRL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case srl_e:       _E = read_mem(NativeCPC, addr); _E = SRL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case srl_h:       _H = read_mem(NativeCPC, addr); _H = SRL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case srl_l:       _L = read_mem(NativeCPC, addr); _L = SRL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case srl_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SRL(Z80, b)); } break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ddcb, 2, bOpCode));
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_ed(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_ed
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tPSG* PSG = NativeCPC->PSG;
tFDC* FDC = NativeCPC->FDC;
tUChar bOpCode;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_ed);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ed, 0, 0));
  bOpCode = read_mem(NativeCPC, _PC++);

  Z80->iCycleCount += cc_ed[bOpCode];
  _R++;

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ed, 1, bOpCode));

  switch (bOpCode)
  {
    case adc_hl_bc:   ADC16(BC); break;
    case adc_hl_de:   ADC16(DE); break;
    case adc_hl_hl:   ADC16(HL); break;
    case adc_hl_sp:   ADC16(SP); break;
    case cpd:         CPD; break;
    case cpdr:        CPDR; break;
    case cpi:         CPI; break;
    case cpir:        CPIR; break;
    case ed_00:       break;
    case ed_01:       break;
    case ed_02:       break;
    case ed_03:       break;
    case ed_04:       break;
    case ed_05:       break;
    case ed_06:       break;
    case ed_07:       break;
    case ed_08:       break;
    case ed_09:       break;
    case ed_0a:       break;
    case ed_0b:       break;
    case ed_0c:       break;
    case ed_0d:       break;
    case ed_0e:       break;
    case ed_0f:       break;
    case ed_10:       break;
    case ed_11:       break;
    case ed_12:       break;
    case ed_13:       break;
    case ed_14:       break;
    case ed_15:       break;
    case ed_16:       break;
    case ed_17:       break;
    case ed_18:       break;
    case ed_19:       break;
    case ed_1a:       break;
    case ed_1b:       break;
    case ed_1c:       break;
    case ed_1d:       break;
    case ed_1e:       break;
    case ed_1f:       break;
    case ed_20:       break;
    case ed_21:       break;
    case ed_22:       break;
    case ed_23:       break;
    case ed_24:       break;
    case ed_25:       break;
    case ed_26:       break;
    case ed_27:       break;
    case ed_28:       break;
    case ed_29:       break;
    case ed_2a:       break;
    case ed_2b:       break;
    case ed_2c:       break;
    case ed_2d:       break;
    case ed_2e:       break;
    case ed_2f:       break;
    case ed_30:       break;
    case ed_31:       break;
    case ed_32:       break;
    case ed_33:       break;
    case ed_34:       break;
    case ed_35:       break;
    case ed_36:       break;
    case ed_37:       break;
    case ed_38:       break;
    case ed_39:       break;
    case ed_3a:       break;
    case ed_3b:       break;
    case ed_3c:       break;
    case ed_3d:       break;
    case ed_3e:       break;
    case ed_3f:       break;
    case ed_77:       break;
    case ed_7f:       break;
    case ed_80:       break;
    case ed_81:       break;
    case ed_82:       break;
    case ed_83:       break;
    case ed_84:       break;
    case ed_85:       break;
    case ed_86:       break;
    case ed_87:       break;
    case ed_88:       break;
    case ed_89:       break;
    case ed_8a:       break;
    case ed_8b:       break;
    case ed_8c:       break;
    case ed_8d:       break;
    case ed_8e:       break;
    case ed_8f:       break;
    case ed_90:       break;
    case ed_91:       break;
    case ed_92:       break;
    case ed_93:       break;
    case ed_94:       break;
    case ed_95:       break;
    case ed_96:       break;
    case ed_97:       break;
    case ed_98:       break;
    case ed_99:       break;
    case ed_9a:       break;
    case ed_9b:       break;
    case ed_9c:       break;
    case ed_9d:       break;
    case ed_9e:       break;
    case ed_9f:       break;
    case ed_a4:       break;
    case ed_a5:       break;
    case ed_a6:       break;
    case ed_a7:       break;
    case ed_ac:       break;
    case ed_ad:       break;
    case ed_ae:       break;
    case ed_af:       break;
    case ed_b4:       break;
    case ed_b5:       break;
    case ed_b6:       break;
    case ed_b7:       break;
    case ed_bc:       break;
    case ed_bd:       break;
    case ed_be:       break;
    case ed_bf:       break;
    case ed_c0:       break;
    case ed_c1:       break;
    case ed_c2:       break;
    case ed_c3:       break;
    case ed_c4:       break;
    case ed_c5:       break;
    case ed_c6:       break;
    case ed_c7:       break;
    case ed_c8:       break;
    case ed_c9:       break;
    case ed_ca:       break;
    case ed_cb:       break;
    case ed_cc:       break;
    case ed_cd:       break;
    case ed_ce:       break;
    case ed_cf:       break;
    case ed_d0:       break;
    case ed_d1:       break;
    case ed_d2:       break;
    case ed_d3:       break;
    case ed_d4:       break;
    case ed_d5:       break;
    case ed_d6:       break;
    case ed_d7:       break;
    case ed_d8:       break;
    case ed_d9:       break;
    case ed_da:       break;
    case ed_db:       break;
    case ed_dc:       break;
    case ed_dd:       break;
    case ed_de:       break;
    case ed_df:       break;
    case ed_e0:       break;
    case ed_e1:       break;
    case ed_e2:       break;
    case ed_e3:       break;
    case ed_e4:       break;
    case ed_e5:       break;
    case ed_e6:       break;
    case ed_e7:       break;
    case ed_e8:       break;
    case ed_e9:       break;
    case ed_ea:       break;
    case ed_eb:       break;
    case ed_ec:       break;
    case ed_ed:       break;
    case ed_ee:       break;
    case ed_ef:       break;
    case ed_f0:       break;
    case ed_f1:       break;
    case ed_f2:       break;
    case ed_f3:       break;
    case ed_f4:       break;
    case ed_f5:       break;
    case ed_f6:       break;
    case ed_f7:       break;
    case ed_f8:       break;
    case ed_f9:       break;
    case ed_fa:       break;
    case ed_fb:       break;
    case ed_fc:       break;
    case ed_fd:       break;
    case ed_fe:       break;
    case ed_ff:       break;
    case im_0:        _IM = 0; break;
    case im_0_1:      _IM = 0; break;
    case im_0_2:      _IM = 0; break;
    case im_0_3:      _IM = 0; break;
    case im_1:        _IM = 1; break;
    case im_1_1:      _IM = 1; break;
    case im_2:        _IM = 2; break;
    case im_2_1:      _IM = 2; break;
    case ind:         { Z80_WAIT_STATES Z80->iCycleCount = Iy_;} IND; break;
    case indr:        { Z80_WAIT_STATES Z80->iCycleCount = Iy_;} INDR; break;
    case ini:         { Z80_WAIT_STATES Z80->iCycleCount = Iy_;} INI; break;
    case inir:        { Z80_WAIT_STATES Z80->iCycleCount = Iy_;} INIR; break;
    case in_0_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} { tUChar res = (tUChar)z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[res]; } break;
    case in_a_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _A = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_A]; break;
    case in_b_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _B = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_B]; break;
    case in_c_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _C = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_C]; break;
    case in_d_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _D = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_D]; break;
    case in_e_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _E = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_E]; break;
    case in_h_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _H = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_H]; break;
    case in_l_c:      { Z80_WAIT_STATES Z80->iCycleCount = Ix_;} _L = z80_IN_handler(NativeCPC, Z80->Regs.BC); _F = (_F & Cflag) | Z80->SZP[_L]; break;
    case ldd:         LDD; Z80->iWSAdjust++; break;
    case lddr:        LDDR; Z80->iWSAdjust++; break;
    case ldi:         LDI; Z80->iWSAdjust++; break;
    case ldir:        LDIR; Z80->iWSAdjust++; break;
    case ld_a_i:      _A = _I; _F = (_F & Cflag) | Z80->SZ[_A] | _IFF2; Z80->iWSAdjust++; break;
    case ld_a_r:      _A = (_R & 0x7f) | _Rb7; _F = (_F & Cflag) | Z80->SZ[_A] | _IFF2; Z80->iWSAdjust++; break;
    case ld_EDbc_mword:  LD16_MEM(BC); break;
    case ld_EDde_mword:  LD16_MEM(DE); break;
    case ld_EDhl_mword:  LD16_MEM(HL); break;
    case ld_EDmword_bc:  LDMEM_16(BC); break;
    case ld_EDmword_de:  LDMEM_16(DE); break;
    case ld_EDmword_hl:  LDMEM_16(HL); break;
    case ld_EDmword_sp:  LDMEM_16(SP); break;
    case ld_EDsp_mword:  LD16_MEM(SP); break;
    case ld_i_a:      _I = _A; Z80->iWSAdjust++; break;
    case ld_r_a:      _R = _A; _Rb7 = _A & 0x80; Z80->iWSAdjust++; break;
    case neg:         NEG; break;
    case neg_1:       NEG; break;
    case neg_2:       NEG; break;
    case neg_3:       NEG; break;
    case neg_4:       NEG; break;
    case neg_5:       NEG; break;
    case neg_6:       NEG; break;
    case neg_7:       NEG; break;
    case otdr:        { Z80_WAIT_STATES Z80->iCycleCount = Oy_;} OTDR; break;
    case otir:        { Z80_WAIT_STATES Z80->iCycleCount = Oy_;} OTIR; break;
    case outd:        { Z80_WAIT_STATES Z80->iCycleCount = Oy_;} OUTD; break;
    case outi:        { Z80_WAIT_STATES Z80->iCycleCount = Oy_;} OUTI; break;
    case out_c_0:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, 0); break;
    case out_c_a:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _A); break;
    case out_c_b:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _B); break;
    case out_c_c:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _C); break;
    case out_c_d:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _D); break;
    case out_c_e:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _E); break;
    case out_c_h:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _H); break;
    case out_c_l:     { Z80_WAIT_STATES Z80->iCycleCount = Ox_;} z80_OUT_handler(NativeCPC, Z80->Regs.BC, _L); break;
    case reti:        _IFF1 = _IFF2; RET; break;
    case reti_1:      _IFF1 = _IFF2; RET; break;
    case reti_2:      _IFF1 = _IFF2; RET; break;
    case reti_3:      _IFF1 = _IFF2; RET; break;
    case retn:        _IFF1 = _IFF2; RET; break;
    case retn_1:      _IFF1 = _IFF2; RET; break;
    case retn_2:      _IFF1 = _IFF2; RET; break;
    case retn_3:      _IFF1 = _IFF2; RET; break;
    case rld:         RLD; break;
    case rrd:         RRD; break;
    case sbc_hl_bc:   SBC16(BC); break;
    case sbc_hl_de:   SBC16(DE); break;
    case sbc_hl_hl:   SBC16(HL); break;
    case sbc_hl_sp:   SBC16(SP); break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_ed, 2, bOpCode));
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_fd(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_fd
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tPSG* PSG = NativeCPC->PSG;
tFDC* FDC = NativeCPC->FDC;
tUChar bOpCode;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_fd);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fd, 0, 0));

  bOpCode = read_mem(NativeCPC, _PC++);
  Z80->iCycleCount += cc_xy[bOpCode];
  _R++;

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fd, 1, bOpCode));

  switch (bOpCode)
  {
    case adc_a:       ADC(_A); break;
    case adc_b:       ADC(_B); break;
    case adc_byte:    ADC(read_mem(NativeCPC, _PC++)); break;
    case adc_c:       ADC(_C); break;
    case adc_d:       ADC(_D); break;
    case adc_e:       ADC(_E); break;
    case adc_h:       ADC(_IYh); break;
    case adc_l:       ADC(_IYl); break;
    case adc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); ADC(read_mem(NativeCPC, (_IY+o))); } break;
    case add_a:       ADD(_A); break;
    case add_b:       ADD(_B); break;
    case add_byte:    ADD(read_mem(NativeCPC, _PC++)); break;
    case add_c:       ADD(_C); break;
    case add_d:       ADD(_D); break;
    case add_e:       ADD(_E); break;
    case add_h:       ADD(_IYh); break;
    case add_hl_bc:   ADD16(IY, BC); break;
    case add_hl_de:   ADD16(IY, DE); break;
    case add_hl_hl:   ADD16(IY, IY); break;
    case add_hl_sp:   ADD16(IY, SP); break;
    case add_l:       ADD(_IYl); break;
    case add_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); ADD(read_mem(NativeCPC, (_IY+o))); } break;
    case and_a:       AND(_A); break;
    case and_b:       AND(_B); break;
    case and_byte:    AND(read_mem(NativeCPC, _PC++)); break;
    case and_c:       AND(_C); break;
    case and_d:       AND(_D); break;
    case and_e:       AND(_E); break;
    case and_h:       AND(_IYh); break;
    case and_l:       AND(_IYl); break;
    case and_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); AND(read_mem(NativeCPC, (_IY+o))); } break;
    case call:        CALL; break;
    case call_c:      if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_m:      if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_nc:     if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_nz:     if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_p:      if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_pe:     if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_po:     if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case call_z:      if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
    case ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & Cflag) << 4) | (_A & Xflags)) ^ Cflag; break;
    case cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); break;
    case cp_a:        CP(_A); break;
    case cp_b:        CP(_B); break;
    case cp_byte:     CP(read_mem(NativeCPC, _PC++)); break;
    case cp_c:        CP(_C); break;
    case cp_d:        CP(_D); break;
    case cp_e:        CP(_E); break;
    case cp_h:        CP(_IYh); break;
    case cp_l:        CP(_IYl); break;
    case cp_mhl:      { tLong o = signed_read_mem(NativeCPC, _PC++); CP(read_mem(NativeCPC, (_IY+o))); } break;
    case daa:         DAA; break;
    case dec_a:       DEC(_A); break;
    case dec_b:       DEC(_B); break;
    case dec_bc:      _BC--; Z80->iWSAdjust++; break;
    case dec_c:       DEC(_C); break;
    case dec_d:       DEC(_D); break;
    case dec_de:      _DE--; Z80->iWSAdjust++; break;
    case dec_e:       DEC(_E); break;
    case dec_h:       DEC(_IYh); break;
    case dec_hl:      _IY--; Z80->iWSAdjust++; break;
    case dec_l:       DEC(_IYl); break;
    case dec_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, (_IY+o)); DEC(b); write_mem(NativeCPC, (tUShort)(_IY+o), b); } break;
    case dec_sp:      _SP--; Z80->iWSAdjust++; break;
    case di:          _IFF1 = _IFF2 = 0; Z80->Regs.EI_issued = 0; break;
    case djnz:        if (--_B) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } break;
    case ei:          Z80->Regs.EI_issued = 2; break;
    case exx:         EXX; break;
    case ex_af_af:    EX(Z80->Regs.AF, Z80->Regs.AFx); break;
    case ex_de_hl:    EX(Z80->Regs.DE, Z80->Regs.HL); break;
    case ex_msp_hl:   EX_SP(IY); Z80->iWSAdjust++; break;
    case halt:        _HALT = 1; _PC--; break;
    case ina:         { Z80_WAIT_STATES Z80->iCycleCount = Ia_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; _A = z80_IN_handler(NativeCPC, p); } break;
    case inc_a:       INC(_A); break;
    case inc_b:       INC(_B); break;
    case inc_bc:      _BC++; Z80->iWSAdjust++; break;
    case inc_c:       INC(_C); break;
    case inc_d:       INC(_D); break;
    case inc_de:      _DE++; Z80->iWSAdjust++; break;
    case inc_e:       INC(_E); break;
    case inc_h:       INC(_IYh); break;
    case inc_hl:      _IY++; Z80->iWSAdjust++; break;
    case inc_l:       INC(_IYl); break;
    case inc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, (_IY+o)); INC(b); write_mem(NativeCPC, (tUShort)(_IY+o), b); } break;
    case inc_sp:      _SP++; Z80->iWSAdjust++; break;
    case jp:          JP; break;
    case jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; break;
    case jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; break;
    case jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; break;
    case jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; break;
    case jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; break;
    case jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; break;
    case jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; break;
    case jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; break;
    case jr:          JR; break;
    case jr_c:        if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_nc:       if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_nz:       if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case jr_z:        if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
    case ld_a_a:      break;
    case ld_a_b:      _A = _B; break;
    case ld_a_byte:   _A = read_mem(NativeCPC, _PC++); break;
    case ld_a_c:      _A = _C; break;
    case ld_a_d:      _A = _D; break;
    case ld_a_e:      _A = _E; break;
    case ld_a_h:      _A = _IYh; break;
    case ld_a_l:      _A = _IYl; break;
    case ld_a_mbc:    _A = read_mem(NativeCPC, _BC); break;
    case ld_a_mde:    _A = read_mem(NativeCPC, _DE); break;
    case ld_a_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _A = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_a_mword:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); _A = read_mem(NativeCPC, addr.w.l); } break;
    case ld_bc_word:  { _C = read_mem(NativeCPC, _PC++); _B = read_mem(NativeCPC, _PC++); } break;
    case ld_b_a:      _B = _A; break;
    case ld_b_b:      break;
    case ld_b_byte:   _B = read_mem(NativeCPC, _PC++); break;
    case ld_b_c:      _B = _C; break;
    case ld_b_d:      _B = _D; break;
    case ld_b_e:      _B = _E; break;
    case ld_b_h:      _B = _IYh; break;
    case ld_b_l:      _B = _IYl; break;
    case ld_b_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _B = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_c_a:      _C = _A; break;
    case ld_c_b:      _C = _B; break;
    case ld_c_byte:   _C = read_mem(NativeCPC, _PC++); break;
    case ld_c_c:      break;
    case ld_c_d:      _C = _D; break;
    case ld_c_e:      _C = _E; break;
    case ld_c_h:      _C = _IYh; break;
    case ld_c_l:      _C = _IYl; break;
    case ld_c_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _C = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_de_word:  { _E = read_mem(NativeCPC, _PC++); _D = read_mem(NativeCPC, _PC++); } break;
    case ld_d_a:      _D = _A; break;
    case ld_d_b:      _D = _B; break;
    case ld_d_byte:   _D = read_mem(NativeCPC, _PC++); break;
    case ld_d_c:      _D = _C; break;
    case ld_d_d:      break;
    case ld_d_e:      _D = _E; break;
    case ld_d_h:      _D = _IYh; break;
    case ld_d_l:      _D = _IYl; break;
    case ld_d_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _D = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_e_a:      _E = _A; break;
    case ld_e_b:      _E = _B; break;
    case ld_e_byte:   _E = read_mem(NativeCPC, _PC++); break;
    case ld_e_c:      _E = _C; break;
    case ld_e_d:      _E = _D; break;
    case ld_e_e:      break;
    case ld_e_h:      _E = _IYh; break;
    case ld_e_l:      _E = _IYl; break;
    case ld_e_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _E = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_hl_mword: LD16_MEM(IY); break;
    case ld_hl_word:  { _IYl = read_mem(NativeCPC, _PC++); _IYh = read_mem(NativeCPC, _PC++); } break;
    case ld_h_a:      _IYh = _A; break;
    case ld_h_b:      _IYh = _B; break;
    case ld_h_byte:   _IYh = read_mem(NativeCPC, _PC++); break;
    case ld_h_c:      _IYh = _C; break;
    case ld_h_d:      _IYh = _D; break;
    case ld_h_e:      _IYh = _E; break;
    case ld_h_h:      break;
    case ld_h_l:      _IYh = _IYl; break;
    case ld_h_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _H = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_l_a:      _IYl = _A; break;
    case ld_l_b:      _IYl = _B; break;
    case ld_l_byte:   _IYl = read_mem(NativeCPC, _PC++); break;
    case ld_l_c:      _IYl = _C; break;
    case ld_l_d:      _IYl = _D; break;
    case ld_l_e:      _IYl = _E; break;
    case ld_l_h:      _IYl = _IYh; break;
    case ld_l_l:      break;
    case ld_l_mhl:    { tLong o = signed_read_mem(NativeCPC, _PC++); _L = read_mem(NativeCPC, (_IY+o)); } break;
    case ld_mbc_a:    write_mem(NativeCPC, _BC, _A); break;
    case ld_mde_a:    write_mem(NativeCPC, _DE, _A); break;
    case ld_mhl_a:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _A); } break;
    case ld_mhl_b:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _B); } break;
    case ld_mhl_byte: { tLong o = signed_read_mem(NativeCPC, _PC++); tULong b = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), b); } break;
    case ld_mhl_c:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _C); } break;
    case ld_mhl_d:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _D); } break;
    case ld_mhl_e:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _E); } break;
    case ld_mhl_h:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _H); } break;
    case ld_mhl_l:    { tLong o = signed_read_mem(NativeCPC, _PC++); write_mem(NativeCPC, (_IY+o), _L); } break;
    case ld_mword_a:  { tRegister addr; addr.b.l = read_mem(NativeCPC, _PC++); addr.b.h = read_mem(NativeCPC, _PC++); write_mem(NativeCPC, addr.w.l, _A); } break;
    case ld_mword_hl: LDMEM_16(IY); break;
    case ld_pc_hl:    _PC = _IY; break;
    case ld_sp_hl:    _SP = _IY; Z80->iWSAdjust++; break;
    case ld_sp_word:  Z80->Regs.SP.b.l = read_mem(NativeCPC, _PC++); Z80->Regs.SP.b.h = read_mem(NativeCPC, _PC++); break;
    case nop:         break;
    case or_a:        OR(_A); break;
    case or_b:        OR(_B); break;
    case or_byte:     OR(read_mem(NativeCPC, _PC++)); break;
    case or_c:        OR(_C); break;
    case or_d:        OR(_D); break;
    case or_e:        OR(_E); break;
    case or_h:        OR(_IYh); break;
    case or_l:        OR(_IYl); break;
    case or_mhl:      { tLong o = signed_read_mem(NativeCPC, _PC++); OR(read_mem(NativeCPC, (_IY+o))); } break;
    case outa:        { Z80_WAIT_STATES Z80->iCycleCount = Oa_;} { tRegister p; p.b.l = read_mem(NativeCPC, _PC++); p.b.h = _A; z80_OUT_handler(NativeCPC, p, _A); } break;
    case pfx_cb:      z80_pfx_fdcb(NativeCPC); break;
    case pfx_dd:      z80_pfx_dd(NativeCPC); break;
    case pfx_ed:      z80_pfx_ed(NativeCPC); break;
    case pfx_fd:      z80_pfx_fd(NativeCPC); break;
    case pop_af:      POP(AF); break;
    case pop_bc:      POP(BC); break;
    case pop_de:      POP(DE); break;
    case pop_hl:      POP(IY); break;
    case push_af:     PUSH(AF); break;
    case push_bc:     PUSH(BC); break;
    case push_de:     PUSH(DE); break;
    case push_hl:     PUSH(IY); break;
    case ret:         RET; break;
    case ret_c:       if (_F & Cflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_m:       if (_F & Sflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_nc:      if (!(_F & Cflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_nz:      if (!(_F & Zflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_p:       if (!(_F & Sflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_pe:      if (_F & Pflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_po:      if (!(_F & Pflag)) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case ret_z:       if (_F & Zflag) { Z80->iCycleCount += cc_ex[bOpCode]; RET } else { Z80->iWSAdjust++; } ; break;
    case rla:         RLA; break;
    case rlca:        RLCA; break;
    case rra:         RRA; break;
    case rrca:        RRCA; break;
    case rst00:       RST(0x0000); break;
    case rst08:       RST(0x0008); break;
    case rst10:       RST(0x0010); break;
    case rst18:       RST(0x0018); break;
    case rst20:       RST(0x0020); break;
    case rst28:       RST(0x0028); break;
    case rst30:       RST(0x0030); break;
    case rst38:       RST(0x0038); break;
    case sbc_a:       SBC(_A); break;
    case sbc_b:       SBC(_B); break;
    case sbc_byte:    SBC(read_mem(NativeCPC, _PC++)); break;
    case sbc_c:       SBC(_C); break;
    case sbc_d:       SBC(_D); break;
    case sbc_e:       SBC(_E); break;
    case sbc_h:       SBC(_IYh); break;
    case sbc_l:       SBC(_IYl); break;
    case sbc_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); SBC(read_mem(NativeCPC, (_IY+o))); } break;
    case scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); break;
    case sub_a:       SUB(_A); break;
    case sub_b:       SUB(_B); break;
    case sub_byte:    SUB(read_mem(NativeCPC, _PC++)); break;
    case sub_c:       SUB(_C); break;
    case sub_d:       SUB(_D); break;
    case sub_e:       SUB(_E); break;
    case sub_h:       SUB(_IYh); break;
    case sub_l:       SUB(_IYl); break;
    case sub_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); SUB(read_mem(NativeCPC, (_IY+o))); } break;
    case xor_a:       XOR(_A); break;
    case xor_b:       XOR(_B); break;
    case xor_byte:    XOR(read_mem(NativeCPC, _PC++)); break;
    case xor_c:       XOR(_C); break;
    case xor_d:       XOR(_D); break;
    case xor_e:       XOR(_E); break;
    case xor_h:       XOR(_IYh); break;
    case xor_l:       XOR(_IYl); break;
    case xor_mhl:     { tLong o = signed_read_mem(NativeCPC, _PC++); XOR(read_mem(NativeCPC, (_IY+o))); } break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fd, 2, bOpCode));
}
/*---------------------------------------------------------------------*/


static tVoid z80_pfx_fdcb(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  z80_pfx_fdcb
 *
 ***********************************************************************/
{
tZ80* Z80 = NativeCPC->Z80;
tULong addr;
tUChar bOpCode;
tLong o;

  PROFILE_ADD_NATIVE(PROFILE_z80_pfx_fdcb);

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fdcb, 0, 0));

  o = signed_read_mem(NativeCPC, _PC++); // offset
  addr = _IY + o;
  bOpCode = read_mem(NativeCPC, _PC++);
  Z80->iCycleCount += cc_xycb[bOpCode];

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fdcb, 1, 0));

  switch (bOpCode)
  {
    case bit0_a:
    case bit0_b:
    case bit0_c:
    case bit0_d:
    case bit0_e:
    case bit0_h:
    case bit0_l:
    case bit0_mhl:    BIT_XY(0, read_mem(NativeCPC, addr)); break;
    case bit1_a:
    case bit1_b:
    case bit1_c:
    case bit1_d:
    case bit1_e:
    case bit1_h:
    case bit1_l:
    case bit1_mhl:    BIT_XY(1, read_mem(NativeCPC, addr)); break;
    case bit2_a:
    case bit2_b:
    case bit2_c:
    case bit2_d:
    case bit2_e:
    case bit2_h:
    case bit2_l:
    case bit2_mhl:    BIT_XY(2, read_mem(NativeCPC, addr)); break;
    case bit3_a:
    case bit3_b:
    case bit3_c:
    case bit3_d:
    case bit3_e:
    case bit3_h:
    case bit3_l:
    case bit3_mhl:    BIT_XY(3, read_mem(NativeCPC, addr)); break;
    case bit4_a:
    case bit4_b:
    case bit4_c:
    case bit4_d:
    case bit4_e:
    case bit4_h:
    case bit4_l:
    case bit4_mhl:    BIT_XY(4, read_mem(NativeCPC, addr)); break;
    case bit5_a:
    case bit5_b:
    case bit5_c:
    case bit5_d:
    case bit5_e:
    case bit5_h:
    case bit5_l:
    case bit5_mhl:    BIT_XY(5, read_mem(NativeCPC, addr)); break;
    case bit6_a:
    case bit6_b:
    case bit6_c:
    case bit6_d:
    case bit6_e:
    case bit6_h:
    case bit6_l:
    case bit6_mhl:    BIT_XY(6, read_mem(NativeCPC, addr)); break;
    case bit7_a:
    case bit7_b:
    case bit7_c:
    case bit7_d:
    case bit7_e:
    case bit7_h:
    case bit7_l:
    case bit7_mhl:    BIT_XY(7, read_mem(NativeCPC, addr)); break;
    case res0_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(0, _A)); break;
    case res0_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(0, _B)); break;
    case res0_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(0, _C)); break;
    case res0_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(0, _D)); break;
    case res0_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(0, _E)); break;
    case res0_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(0, _H)); break;
    case res0_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(0, _L)); break;
    case res0_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(0, b)); } break;
    case res1_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(1, _A)); break;
    case res1_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(1, _B)); break;
    case res1_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(1, _C)); break;
    case res1_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(1, _D)); break;
    case res1_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(1, _E)); break;
    case res1_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(1, _H)); break;
    case res1_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(1, _L)); break;
    case res1_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(1, b)); } break;
    case res2_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(2, _A)); break;
    case res2_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(2, _B)); break;
    case res2_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(2, _C)); break;
    case res2_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(2, _D)); break;
    case res2_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(2, _E)); break;
    case res2_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(2, _H)); break;
    case res2_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(2, _L)); break;
    case res2_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(2, b)); } break;
    case res3_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(3, _A)); break;
    case res3_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(3, _B)); break;
    case res3_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(3, _C)); break;
    case res3_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(3, _D)); break;
    case res3_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(3, _E)); break;
    case res3_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(3, _H)); break;
    case res3_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(3, _L)); break;
    case res3_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(3, b)); } break;
    case res4_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(4, _A)); break;
    case res4_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(4, _B)); break;
    case res4_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(4, _C)); break;
    case res4_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(4, _D)); break;
    case res4_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(4, _E)); break;
    case res4_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(4, _H)); break;
    case res4_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(4, _L)); break;
    case res4_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(4, b)); } break;
    case res5_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(5, _A)); break;
    case res5_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(5, _B)); break;
    case res5_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(5, _C)); break;
    case res5_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(5, _D)); break;
    case res5_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(5, _E)); break;
    case res5_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(5, _H)); break;
    case res5_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(5, _L)); break;
    case res5_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(5, b)); } break;
    case res6_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(6, _A)); break;
    case res6_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(6, _B)); break;
    case res6_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(6, _C)); break;
    case res6_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(6, _D)); break;
    case res6_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(6, _E)); break;
    case res6_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(6, _H)); break;
    case res6_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(6, _L)); break;
    case res6_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(6, b)); } break;
    case res7_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = RES(7, _A)); break;
    case res7_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = RES(7, _B)); break;
    case res7_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = RES(7, _C)); break;
    case res7_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = RES(7, _D)); break;
    case res7_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = RES(7, _E)); break;
    case res7_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = RES(7, _H)); break;
    case res7_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = RES(7, _L)); break;
    case res7_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RES(7, b)); } break;
    case rlc_a:       _A = read_mem(NativeCPC, addr); _A = RLC(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rlc_b:       _B = read_mem(NativeCPC, addr); _B = RLC(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rlc_c:       _C = read_mem(NativeCPC, addr); _C = RLC(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rlc_d:       _D = read_mem(NativeCPC, addr); _D = RLC(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rlc_e:       _E = read_mem(NativeCPC, addr); _E = RLC(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rlc_h:       _H = read_mem(NativeCPC, addr); _H = RLC(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rlc_l:       _L = read_mem(NativeCPC, addr); _L = RLC(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rlc_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RLC(Z80, b)); } break;
    case rl_a:        _A = read_mem(NativeCPC, addr); _A = RL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rl_b:        _B = read_mem(NativeCPC, addr); _B = RL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rl_c:        _C = read_mem(NativeCPC, addr); _C = RL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rl_d:        _D = read_mem(NativeCPC, addr); _D = RL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rl_e:        _E = read_mem(NativeCPC, addr); _E = RL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rl_h:        _H = read_mem(NativeCPC, addr); _H = RL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rl_l:        _L = read_mem(NativeCPC, addr); _L = RL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rl_mhl:      { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RL(Z80, b)); } break;
    case rrc_a:       _A = read_mem(NativeCPC, addr); _A = RRC(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rrc_b:       _B = read_mem(NativeCPC, addr); _B = RRC(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rrc_c:       _C = read_mem(NativeCPC, addr); _C = RRC(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rrc_d:       _D = read_mem(NativeCPC, addr); _D = RRC(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rrc_e:       _E = read_mem(NativeCPC, addr); _E = RRC(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rrc_h:       _H = read_mem(NativeCPC, addr); _H = RRC(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rrc_l:       _L = read_mem(NativeCPC, addr); _L = RRC(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rrc_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RRC(Z80, b)); } break;
    case rr_a:        _A = read_mem(NativeCPC, addr); _A = RR(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case rr_b:        _B = read_mem(NativeCPC, addr); _B = RR(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case rr_c:        _C = read_mem(NativeCPC, addr); _C = RR(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case rr_d:        _D = read_mem(NativeCPC, addr); _D = RR(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case rr_e:        _E = read_mem(NativeCPC, addr); _E = RR(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case rr_h:        _H = read_mem(NativeCPC, addr); _H = RR(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case rr_l:        _L = read_mem(NativeCPC, addr); _L = RR(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case rr_mhl:      { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, RR(Z80, b)); } break;
    case set0_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(0, _A)); break;
    case set0_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(0, _B)); break;
    case set0_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(0, _C)); break;
    case set0_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(0, _D)); break;
    case set0_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(0, _E)); break;
    case set0_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(0, _H)); break;
    case set0_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(0, _L)); break;
    case set0_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(0, b)); } break;
    case set1_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(1, _A)); break;
    case set1_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(1, _B)); break;
    case set1_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(1, _C)); break;
    case set1_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(1, _D)); break;
    case set1_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(1, _E)); break;
    case set1_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(1, _H)); break;
    case set1_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(1, _L)); break;
    case set1_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(1, b)); } break;
    case set2_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(2, _A)); break;
    case set2_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(2, _B)); break;
    case set2_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(2, _C)); break;
    case set2_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(2, _D)); break;
    case set2_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(2, _E)); break;
    case set2_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(2, _H)); break;
    case set2_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(2, _L)); break;
    case set2_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(2, b)); } break;
    case set3_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(3, _A)); break;
    case set3_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(3, _B)); break;
    case set3_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(3, _C)); break;
    case set3_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(3, _D)); break;
    case set3_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(3, _E)); break;
    case set3_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(3, _H)); break;
    case set3_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(3, _L)); break;
    case set3_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(3, b)); } break;
    case set4_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(4, _A)); break;
    case set4_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(4, _B)); break;
    case set4_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(4, _C)); break;
    case set4_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(4, _D)); break;
    case set4_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(4, _E)); break;
    case set4_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(4, _H)); break;
    case set4_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(4, _L)); break;
    case set4_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(4, b)); } break;
    case set5_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(5, _A)); break;
    case set5_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(5, _B)); break;
    case set5_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(5, _C)); break;
    case set5_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(5, _D)); break;
    case set5_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(5, _E)); break;
    case set5_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(5, _H)); break;
    case set5_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(5, _L)); break;
    case set5_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(5, b)); } break;
    case set6_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(6, _A)); break;
    case set6_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(6, _B)); break;
    case set6_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(6, _C)); break;
    case set6_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(6, _D)); break;
    case set6_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(6, _E)); break;
    case set6_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(6, _H)); break;
    case set6_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(6, _L)); break;
    case set6_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(6, b)); } break;
    case set7_a:      _A = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _A = SET(7, _A)); break;
    case set7_b:      _B = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _B = SET(7, _B)); break;
    case set7_c:      _C = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _C = SET(7, _C)); break;
    case set7_d:      _D = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _D = SET(7, _D)); break;
    case set7_e:      _E = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _E = SET(7, _E)); break;
    case set7_h:      _H = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _H = SET(7, _H)); break;
    case set7_l:      _L = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, _L = SET(7, _L)); break;
    case set7_mhl:    { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SET(7, b)); } break;
    case sla_a:       _A = read_mem(NativeCPC, addr); _A = SLA(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sla_b:       _B = read_mem(NativeCPC, addr); _B = SLA(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sla_c:       _C = read_mem(NativeCPC, addr); _C = SLA(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sla_d:       _D = read_mem(NativeCPC, addr); _D = SLA(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sla_e:       _E = read_mem(NativeCPC, addr); _E = SLA(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sla_h:       _H = read_mem(NativeCPC, addr); _H = SLA(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sla_l:       _L = read_mem(NativeCPC, addr); _L = SLA(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sla_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SLA(Z80, b)); } break;
    case sll_a:       _A = read_mem(NativeCPC, addr); _A = SLL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sll_b:       _B = read_mem(NativeCPC, addr); _B = SLL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sll_c:       _C = read_mem(NativeCPC, addr); _C = SLL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sll_d:       _D = read_mem(NativeCPC, addr); _D = SLL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sll_e:       _E = read_mem(NativeCPC, addr); _E = SLL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sll_h:       _H = read_mem(NativeCPC, addr); _H = SLL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sll_l:       _L = read_mem(NativeCPC, addr); _L = SLL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sll_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SLL(Z80, b)); } break;
    case sra_a:       _A = read_mem(NativeCPC, addr); _A = SRA(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case sra_b:       _B = read_mem(NativeCPC, addr); _B = SRA(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case sra_c:       _C = read_mem(NativeCPC, addr); _C = SRA(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case sra_d:       _D = read_mem(NativeCPC, addr); _D = SRA(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case sra_e:       _E = read_mem(NativeCPC, addr); _E = SRA(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case sra_h:       _H = read_mem(NativeCPC, addr); _H = SRA(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case sra_l:       _L = read_mem(NativeCPC, addr); _L = SRA(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case sra_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SRA(Z80, b)); } break;
    case srl_a:       _A = read_mem(NativeCPC, addr); _A = SRL(Z80, _A); write_mem(NativeCPC, addr, _A); break;
    case srl_b:       _B = read_mem(NativeCPC, addr); _B = SRL(Z80, _B); write_mem(NativeCPC, addr, _B); break;
    case srl_c:       _C = read_mem(NativeCPC, addr); _C = SRL(Z80, _C); write_mem(NativeCPC, addr, _C); break;
    case srl_d:       _D = read_mem(NativeCPC, addr); _D = SRL(Z80, _D); write_mem(NativeCPC, addr, _D); break;
    case srl_e:       _E = read_mem(NativeCPC, addr); _E = SRL(Z80, _E); write_mem(NativeCPC, addr, _E); break;
    case srl_h:       _H = read_mem(NativeCPC, addr); _H = SRL(Z80, _H); write_mem(NativeCPC, addr, _H); break;
    case srl_l:       _L = read_mem(NativeCPC, addr); _L = SRL(Z80, _L); write_mem(NativeCPC, addr, _L); break;
    case srl_mhl:     { tULong b = read_mem(NativeCPC, addr); write_mem(NativeCPC, addr, SRL(Z80, b)); } break;
  }

  SHOWTRACE(TRACE_DATA(TRACE_FN_z80_pfx_fdcb, 2, 0));
}
/*---------------------------------------------------------------------*/


static tULong z80_IN_handler(tNativeCPC* NativeCPC,
                             tRegister port)
/***********************************************************************
 *
 *  z80_IN_handler
 *
 ***********************************************************************/
{
tULong ret_val;

  PROFILE_ADD_NATIVE(PROFILE_z80_IN_handler);

  ret_val = 0xff; // default return value

  // CRTC ----------------------------------------------------------------------
  if (!(port.b.h & 0x40)) // CRTC chip select?
  {
    tCRTC* CRTC = NativeCPC->CRTC;

    if ((port.b.h & 3) == 3) // read CRTC register?
    {
      if ((CRTC->reg_select > 11) && (CRTC->reg_select < 16)) // valid range?
      {
        ret_val = CRTC->registers[CRTC->reg_select];
      }
      else if (CRTC->reg_select == 16)
      {
      	ret_val = ((CRTC->start_addr + CRTC->lpen_offset) >> 8) & 0x3f;
      }
      else if (CRTC->reg_select == 17)
      {
        if (NativeCPC->lightgun_random_crtc)
        {
          // return random value different from previous one
          ret_val = (tUChar)NativeCPC->cycle_count;
        }
        else
        {
        	ret_val = (CRTC->start_addr + CRTC->lpen_offset) & 0xff;
        }
      }
      else
      {
        ret_val = 0; // write only registers return 0
      }
    }
  }

  // PPI -----------------------------------------------------------------------
  else if (!(port.b.h & 0x08)) // PPI chip select?
  {
    tPPI* PPI = NativeCPC->PPI;
    tPSG* PSG = NativeCPC->PSG;
    tCRTC* CRTC = NativeCPC->CRTC;
    tULong ppi_port;
    tULong direction;
    tULong val;

    ppi_port = port.b.h & 3;

    switch (ppi_port)
    {
      case 0: // read from port A?
      {
        if (PPI->control & 0x10) // port A set to input?
        {
          if ((PSG->control & 0xc0) == 0x40) // PSG control set to read?
          {
            if (PSG->reg_select < 16) // within valid range?
            {
              if (PSG->reg_select == 14) // PSG port A?
              {
                if (!(PSG->RegisterAY.Index[7] & 0x40)) // port A in input mode?
                {
                  ret_val = NativeCPC->keyboard_matrix[NativeCPC->keyboard_line & 0x0f]; // read keyboard matrix node status

                  if (NativeCPC->lightgun_counter)
                  {
                    if ((NativeCPC->keyboard_line & 0x0f) == NativeCPC->lightgun_beam_key_line)
                    {
                      ret_val &= NativeCPC->lightgun_beam_key_mask;
#ifdef _DEBUG
                      NativeCPC->lightgun_debug_counter++;
#endif /* _DEBUG */
                    }
                  }
                }
                else
                {
                  ret_val = PSG->RegisterAY.Index[14] & (NativeCPC->keyboard_matrix[NativeCPC->keyboard_line & 0x0f]); // return last value w/ logic AND of input
                }
              }
              else if (PSG->reg_select == 15) // PSG port B?
              {
                if ((PSG->RegisterAY.Index[7] & 0x80)) // port B in output mode?
                {
                  ret_val = PSG->RegisterAY.Index[15]; // return stored value
                }
              }
              else
              {
                ret_val = PSG->RegisterAY.Index[PSG->reg_select]; // read PSG register
              }
            }
          }
        }
        else
        {
          ret_val = PPI->portA; // return last programmed value
        }
      }
      break;

      case 1: // read from port B?
      {
        if (PPI->control & 2) // port B set to input?
        {
#ifdef ENABLE_TAPE
          ret_val = PSG->bTapeLevel | // Tape level when reading (TAPE_LEVEL_HIGH or TAPE_LEVEL_LOW)
#else /* ENABLE_TAPE */
          ret_val =
#endif /* ENABLE_TAPE */
#ifdef ENABLE_PRINTER
                    (NativeCPC->printer ? 0 : 0x40) | // ready line of connected printer
#else /* ENABLE_PRINTER */
                    0x40 |
#endif /* ENABLE_PRINTER */
                    (NativeCPC->jumpers & (CPC_BRAND_MASK | CPC_JUMPER_50HZ)) |
                    (CRTC->flags & VS_flag); // VSYNC status (VS_flag=0x01)
        }
        else
        {
          ret_val = PPI->portB; // return last programmed value
        }
      }
      break;

      case 2: // read from port C?
      {
        direction = PPI->control & 9; // isolate port C directions
        ret_val = PPI->portC; // default to last programmed value

        if (direction) // either half set to input?
        {
          if (direction & 8) // upper half set to input?
          {
            ret_val &= 0x0f; // blank out upper half
            val = PPI->portC & 0xc0; // isolate PSG control bits

            if (val == 0xc0) // PSG specify register?
            {
              val = 0x80; // change to PSG write register
            }

            ret_val |= val | 0x20; // casette write data is always set
            if (NativeCPC->tape_motor)
            {
              ret_val |= 0x10; // set the bit if the tape motor is running
            }
          }
          if (!(direction & 1)) // lower half set to output?
          {
            ret_val |= 0x0f; // invalid - set all bits
          }
        }
      }
      break;
    }
  }

  //----------------------------------------------------------------------------
  else if ((port.b.h == 0xfb) && (!(port.b.l & 0x80))) // FDC?
  {
    if (!(port.b.l & 0x01)) // FDC status register?
    {
      PROFILE_ADD_NATIVE(PROFILE_fdc_read_status);
      ret_val = fdc_read_status(NativeCPC->FDC);
    }
    else // FDC data register
    {
      PROFILE_ADD_NATIVE(PROFILE_fdc_read_data);
      ret_val = fdc_read_data(NativeCPC->FDC);
    }
  }

  return ret_val;
}
/*---------------------------------------------------------------------*/


static tVoid z80_OUT_handler(tNativeCPC* NativeCPC,
                             tRegister port,
                             tULong val)
/***********************************************************************
 *
 *  z80_OUT_handler
 *
 ***********************************************************************/
{
  PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler);

  // Gate Array ----------------------------------------------------------------
  if ((port.b.h & 0xc0) == 0x40) // GA chip select?
  {
    PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler_GA);
    tGateArray* GateArray = NativeCPC->GateArray;
    tCRTC* CRTC = NativeCPC->CRTC;

    switch (val >> 6)
    {
      case 0: // select pen
      {
        GateArray->pen = val & 0x10 ? 0x10 : val & 0x0f; // if bit 5 is set, pen indexes the border colour
      }
      break;

      case 1: // set colour
      {
        GateArray->palette[GateArray->pen] = NativeCPC->colours[val & 0x1f]; // isolate colour value

        if (CRTC->current_mode == 2) // mode 2
        {
          tULong indexed_color;
          tULong intensity0 = 0;
          tULong intensity1 = 0;
          tULong shadowIndex = 32;
          tULong loop;

          for (loop=0; loop < 32; loop++)
          {
            if (NativeCPC->colours[loop] == GateArray->palette[0])
            {
              intensity0 = NativeCPC->active_colours[loop].intensity;
            }
            else if (NativeCPC->colours[loop] == GateArray->palette[1])
            {
              intensity1 = NativeCPC->active_colours[loop].intensity;
            }
          }
          indexed_color = GateArray->palette[intensity0 > intensity1 ? 0 : 1];

          for (loop=0; loop < 32; loop++)
          {
            if (NativeCPC->colours[loop] == indexed_color)
            {
              shadowIndex = NativeCPC->active_colours[loop].shadow_index;
              break;
            }
          }

          // Prepare medium color
          GateArray->palette[2] = shadowIndex > 31 ? GateArray->palette[0] : NativeCPC->colours[shadowIndex];
        }
      }
      break;

      case 2: // set mode
      {
        GateArray->ROM_config = val;
        CRTC->requested_mode = val & 0x03;
        CRTC->RequestedHandler = CRTC->DrawHandler[val & 0x03];
        ga_memory_manager(NativeCPC);
        if (val & 0x10) // delay Z80 interrupt?
        {
          NativeCPC->Z80->Regs.int_pending = 0;  // clear pending interrupts
          GateArray->sl_count = 0; // reset GA scanline counter
        }
      }
      break;

      case 3: // set memory configuration
      {
        GateArray->RAM_config = val;
        ga_memory_manager(NativeCPC);
        break;
      }
    }
  }

  // CRTC ----------------------------------------------------------------------
  else if (!(port.b.h & 0x40)) // CRTC chip select?
  {
    PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler_CRTC);
    tVDU* VDU = NativeCPC->VDU;
    tCRTC* CRTC = NativeCPC->CRTC;
    tULong crtc_port;
    tULong val1;
    tULong val2;

    crtc_port = port.b.h & 3;
    if (crtc_port == 0) // CRTC register select?
    {
      CRTC->reg_select = val;
    }
    else if (crtc_port == 1) // CRTC write data?
    {
      if (CRTC->reg_select < 16) // only registers 0 - 15 can be written to
      {
        CRTC->registers[CRTC->reg_select] = val;

        switch (CRTC->reg_select)
        {
          case 3: // sync width
          {
            CRTC->hsw = val & 0x0f; // isolate horizontal sync width
            VDU->hsw = CRTC->hsw - 2; // GA delays HSYNC by 2 chars
            if (VDU->hsw < 0) // negative value?
            {
              VDU->hsw = 0; // no VDU HSYNC
            }
            else if (VDU->hsw > 4) // HSYNC longer than 4 chars?
            {
              VDU->hsw = 4; // maxium of 4
            }
            CRTC->vsw = val >> 4; // isolate vertical sync width
            if (!CRTC->vsw)
            {
              CRTC->vsw = 16; // 0 = width of 16
            }
          }
          break;

          case 5: // vertical total adjust
          {
            CRTC->vt_adjust = val & 0x1f;
          }
          break;

          case 8: // interlace and skew
          {
            CRTC->skew = (val >> 4) & 3; // isolate display timing skew
            if (CRTC->skew == 3) // no output?
            {
              CRTC->skew = 0xff;
            }
          }
          break;

          case 9: // maximum raster count
          {
            CRTC->max_raster = val << 3; // modify value for easier handling
          }
          break;

          case 12: // start address high byte
          case 13: // start address low byte
          {
            val1 = CRTC->registers[12] & 0x3f;
            CRTC->start_addr = (val1 << 8) + CRTC->registers[13];
            val2 = val1 & 0x0f; // isolate screen size
            val1 = (val1 << 1) & 0x60; // isolate CPC RAM bank
            val2 |= val1; // combine
            CRTC->requested_addr = (CRTC->registers[13] + (val2 << 8)) << 1;
          }
          break;
        }
      }
    }
  }

  // ROM select ----------------------------------------------------------------
  else if (!(port.b.h & 0x20)) // ROM select?
  {
    PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler_ROM);
    tGateArray* GateArray = NativeCPC->GateArray;

    GateArray->upper_ROM = val;
    NativeCPC->pbExpansionROM = NativeCPC->memmap_ROM[val];
    if (NativeCPC->pbExpansionROM == cNull) // selected expansion ROM not present?
    {
      NativeCPC->pbExpansionROM = NativeCPC->pbROMhi; // revert to BASIC ROM
    }
    if (!(GateArray->ROM_config & 0x08)) // upper/expansion ROM is enabled?
    {
      NativeCPC->membank_read[3] = NativeCPC->pbExpansionROM; // 'page in' upper/expansion ROM
    }
  }

#ifdef ENABLE_PRINTER
  // printer port --------------------------------------------------------------
  else if (!(port.b.h & 0x10)) // printer port?
  {
    NativeCPC->printer_port = val ^ 0x80; // invert bit 7
  }
#endif /* ENABLE_PRINTER */

  // PPI -----------------------------------------------------------------------
  else if (!(port.b.h & 0x08)) // PPI chip select?
  {
    PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler_PPI);
    tPPI* PPI = NativeCPC->PPI;
    tPSG* PSG = NativeCPC->PSG;
    tULong psg_data;
    tULong bit;

    switch (port.b.h & 3)
    {
      case 0: // write to port A?
      {
        PPI->portA = val;
        if (!(PPI->control & 0x10)) // port A set to output?
        {
          psg_data = val;
          PSG_WRITE;
        }
      }
      break;

      case 1: // write to port B?
      {
        PPI->portB = val;
      }
      break;

      case 2: // write to port C?
      {
        PPI->portC = val;
        if (!(PPI->control & 1)) // output lower half?
        {
          NativeCPC->keyboard_line = val;
        }
        if (!(PPI->control & 8)) // output upper half?
        {
          NativeCPC->tape_motor = val & 0x10; // update tape motor control
          PSG->control = val; // change PSG control
          psg_data = PPI->portA;
          PSG_WRITE;
        }
      }
      break;

      case 3: // modify PPI control
      {
        if (val & 0x80) // change PPI configuration
        {
          PPI->control = val; // update control byte
          PPI->portA = 0; // clear data for all ports
          PPI->portB = 0;
          PPI->portC = 0;
        }
        else // bit manipulation of port C data
        {
          bit = (val >> 1) & 7; // isolate bit to change
          if (val & 1) // set bit?
          {
            PPI->portC |= 1 << bit; // set requested bit
          }
          else
          {
            PPI->portC &= ~(1 << bit); // reset requested bit
          }
          if (!(PPI->control & 1)) // output lower half?
          {
            NativeCPC->keyboard_line = PPI->portC;
          }
          if (!(PPI->control & 8)) // output upper half?
          {
            NativeCPC->tape_motor = PPI->portC & 0x10;
            PSG->control = PPI->portC; // change PSG control
            psg_data = PPI->portA;
            PSG_WRITE;
          }
        }
      }
      break;
    }
  }

  // FDC -----------------------------------------------------------------------
  else if (!(port.b.l & 0x80))
  {
    PROFILE_ADD_NATIVE(PROFILE_z80_OUT_handler_FDC);
    if (port.b.h == 0xfa) // floppy motor control?
    {
      NativeCPC->FDC->motor = val & 0x01;
      NativeCPC->FDC->flags |= STATUSDRVA_flag | STATUSDRVB_flag;
    }
    else if (port.b.h == 0xfb) // FDC data register?
    {
      PROFILE_ADD_NATIVE(PROFILE_fdc_write_data);
      fdc_write_data(NativeCPC,
                     val);
    }
  }
}
/*---------------------------------------------------------------------*/


static tVoid ga_memory_manager(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  ga_memory_manager
 *
 ***********************************************************************/
{
tGateArray* GateArray = NativeCPC->GateArray;
tUChar mem_bank;
tULong n;

  PROFILE_ADD_NATIVE(PROFILE_ga_memory_manager);

  if (NativeCPC->ram_size == 64) // 64KB of RAM?
  {
    mem_bank = 0; // no expansion memory
    GateArray->RAM_config = 0; // the only valid configuration is 0
  }
  else
  {
    mem_bank = (GateArray->RAM_config >> 3) & 7; // extract expansion memory bank
    if (((mem_bank+2)*64) > NativeCPC->ram_size) // selection is beyond available memory?
    {
      mem_bank = 0; // force default mapping
    }
  }

  if (mem_bank != GateArray->RAM_bank) // requested bank is different from the active one?
  {
    GateArray->RAM_bank = mem_bank;
    ga_init_banking(NativeCPC);
  }

  for (n = 0; n < 4; n++) // remap active memory banks
  {
    NativeCPC->membank_read[n] = NativeCPC->membank_config[GateArray->RAM_config & 7][n];
    NativeCPC->membank_write[n] = NativeCPC->membank_config[GateArray->RAM_config & 7][n];
  }

  if (!(GateArray->ROM_config & 0x04)) // lower ROM is enabled?
  {
    NativeCPC->membank_read[0] = NativeCPC->pbROMlo; // 'page in' lower ROM
  }

  if (!(GateArray->ROM_config & 0x08)) // upper/expansion ROM is enabled?
  {
    NativeCPC->membank_read[3] = NativeCPC->pbExpansionROM; // 'page in' upper/expansion ROM
  }
}
/*---------------------------------------------------------------------*/


static tVoid ga_init_banking(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  ga_init_banking
 *
 ***********************************************************************/
{
tUChar *romb0, *romb1, *romb2, *romb3, *romb4, *romb5, *romb6, *romb7;
tUChar* pbRAMbank;

  PROFILE_ADD_NATIVE(PROFILE_ga_init_banking);

  romb0 = NativeCPC->pbRAM;
  romb1 = romb0 + 16384;
  romb2 = romb1 + 16384;
  romb3 = romb2 + 16384;

  pbRAMbank = NativeCPC->pbRAM + ((NativeCPC->GateArray->RAM_bank + 1) * 65536);
  romb4 = pbRAMbank;
  romb5 = romb4 + 16384;
  romb6 = romb5 + 16384;
  romb7 = romb6 + 16384;

  NativeCPC->membank_config[0][0] = romb0;
  NativeCPC->membank_config[0][1] = romb1;
  NativeCPC->membank_config[0][2] = romb2;
  NativeCPC->membank_config[0][3] = romb3;

  NativeCPC->membank_config[1][0] = romb0;
  NativeCPC->membank_config[1][1] = romb1;
  NativeCPC->membank_config[1][2] = romb2;
  NativeCPC->membank_config[1][3] = romb7;

  NativeCPC->membank_config[2][0] = romb4;
  NativeCPC->membank_config[2][1] = romb5;
  NativeCPC->membank_config[2][2] = romb6;
  NativeCPC->membank_config[2][3] = romb7;

  NativeCPC->membank_config[3][0] = romb0;
  NativeCPC->membank_config[3][1] = romb3;
  NativeCPC->membank_config[3][2] = romb2;
  NativeCPC->membank_config[3][3] = romb7;

  NativeCPC->membank_config[4][0] = romb0;
  NativeCPC->membank_config[4][1] = romb4;
  NativeCPC->membank_config[4][2] = romb2;
  NativeCPC->membank_config[4][3] = romb3;

  NativeCPC->membank_config[5][0] = romb0;
  NativeCPC->membank_config[5][1] = romb5;
  NativeCPC->membank_config[5][2] = romb2;
  NativeCPC->membank_config[5][3] = romb3;

  NativeCPC->membank_config[6][0] = romb0;
  NativeCPC->membank_config[6][1] = romb6;
  NativeCPC->membank_config[6][2] = romb2;
  NativeCPC->membank_config[6][3] = romb3;

  NativeCPC->membank_config[7][0] = romb0;
  NativeCPC->membank_config[7][1] = romb7;
  NativeCPC->membank_config[7][2] = romb2;
  NativeCPC->membank_config[7][3] = romb3;
}
/*---------------------------------------------------------------------*/


//
// VIDEO
//

static tVoid video_init(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  video_init
 *
 ***********************************************************************/
{
tCRTC* CRTC = NativeCPC->CRTC;
tVDU* VDU = NativeCPC->VDU;

  CRTC->DrawHandler[0] = (tDrawHandlerPtr)video_draw_mode0;
  CRTC->DrawHandler[1] = (tDrawHandlerPtr)video_draw_mode1;
  if (NativeCPC->prefP->Mode2AntiAliasing == 0)
  {
    CRTC->DrawHandler[2] = (tDrawHandlerPtr)video_draw_mode2;
  }
  else
  {
    CRTC->DrawHandler[2] = (tDrawHandlerPtr)video_draw_mode2_antialiased;
  }
  CRTC->DrawHandler[3] = (tDrawHandlerPtr)video_draw_mode0;

  CRTC->CurrentHandler = CRTC->DrawHandler[CRTC->current_mode];
  CRTC->RequestedHandler = CRTC->DrawHandler[CRTC->requested_mode];

  VDU->palette = NativeCPC->GateArray->palette;
  VDU->pbRAM = NativeCPC->pbRAM;
  VDU->mode0_table = NativeCPC->mode0_table;
  VDU->mode1_table = NativeCPC->mode1_table;
}
/*---------------------------------------------------------------------*/

//#ifdef NO_BORDER_RENDER
//#define VIDEO_DRAW_BORDER
//#else
#define VIDEO_DRAW_BORDER \
{ \
  tULong Pixels; \
  \
  PROFILE_ADD_NATIVE(PROFILE_video_draw_border); \
  \
  Pixels = NativeCPC->GateArray->palette[16]; \
  Pixels |= Pixels << 8; \
  Pixels |= Pixels << 16; \
  \
  *(VDU->scr_current++) = Pixels; /* write four pixels */ \
  *(VDU->scr_current++) = Pixels; /* write four pixels */ \
}
//#endif
//#ifdef SIM
static tVoid video_access_memory(tNativeCPC* NativeCPC,
                                 tULong repeat_count)
//#else
//static tVoid IRAM_ATTR video_access_memory(tNativeCPC* NativeCPC,
//                                 tULong repeat_count)
//#endif
/***********************************************************************
 *
 *  video_access_memory
 *
 ***********************************************************************/
{
tCRTC* CRTC = NativeCPC->CRTC;
tVDU* VDU = NativeCPC->VDU;
tGateArray* GateArray = NativeCPC->GateArray;
tULong flags;

#ifdef _DEBUG
  if (!repeat_count)
  {
    PROFILE_ADD_NATIVE(PROFILE_Debug);
    return;
  }
#endif /* _DEBUG */

  flags = CRTC->flags;

  do
  {
    PROFILE_ADD_NATIVE(PROFILE_video_access_memory_Loop);

    CRTC->char_count++; // update character count (cc)
    VDU->char_count++;

    if (flags & HT_flag) // reached horizontal total on last cc?
    {
      flags &= ~HT_flag;
      CRTC->hsw_active = CRTC->hsw;
      VDU->hsw_active = VDU->hsw;
      CRTC->char_count = 0; // reset cc

      // next raster -------------------------------------------------

      CRTC->raster_count += 8; // advance rc by one

      if (flags & VS_flag) // in VSYNC?
      {
        CRTC->vsw_count++; // update width counter
        if (CRTC->vsw_count >= CRTC->vsw) // reached end of VSYNC?
        {
          flags = (flags & ~VS_flag) | VSf_flag; // reset VSYNC, set 'just finished'
        }
      }

      if (flags & MR_flag) // reached maximum raster address on last rc?
      {
        flags &= ~MR_flag;
        CRTC->raster_count = 0; // reset rc
        if (!(flags & HDT_flag)) // HDISPTMG still on (i.e. R01 > R00)?
        {
          CRTC->addr += CRTC->last_hdisp * 2; // advance CPC screen address to next line
        }
        CRTC->line_count++; // update line count
        CRTC->line_count &= 127; // limit to 7 bits
      }

      if (CRTC->vt_adjust_count) // vertical total adjust active?
      {
        if (--CRTC->vt_adjust_count == 0) // done with vta?
        {
          flags = (flags & ~VSf_flag) | VDT_flag; // enable VDISPTMG
          CRTC->raster_count = 0; // reset rc
          CRTC->line_count = 0; // reset lc
          CRTC->addr = CRTC->requested_addr; // update start of CPC screen address
        }
      }

      if (flags & VT_flag) // reached vertical total on last lc?
      {
        flags &= ~VT_flag;
        if (CRTC->vt_adjust) // do a vertical total adjust?
        {
          CRTC->vt_adjust_count = CRTC->vt_adjust; // init vta counter
        }
        else
        {
          flags = (flags & ~VSf_flag) | VDT_flag; // enable VDISPTMG
          CRTC->raster_count = 0; // reset rc
          CRTC->line_count = 0; // reset lc
          CRTC->addr = CRTC->requested_addr; // update start of CPC screen address
        }
      }

      if (CRTC->raster_count >= CRTC->max_raster) // rc = maximum raster address?
      {
        flags |= MR_flag; // process max raster address on next rc
        if (!CRTC->vt_adjust_count) // no vertical total adjust?
        {
          if (CRTC->line_count >= CRTC->registers[4]) // lc = vertical total?
          {
            flags |= VT_flag; // takes effect on next lc
          }
        }
      }

      if (CRTC->line_count == CRTC->registers[6]) // lc = vertical displayed?
      {
        flags &= ~VDT_flag; // disable VDISPTMG
      }

      if (CRTC->line_count == CRTC->registers[7]) // lc = vertical sync position?
      {
        if (!(flags & (VSf_flag | VS_flag))) // not in VSYNC?
        {
          flags |= VS_flag;
          CRTC->vsw_count = 0; // clear vsw counter
          VDU->vdelay = 2; // GA delays vsync by 2 scanlines
          VDU->vsw_count = 4; // GA vsync is always 4 scanlines long
          GateArray->int_delay = 2; // arm GA two scanlines interrupt delay
        }
      }

      // Decrement lightgun beam duration counter (in raster)
      if (NativeCPC->lightgun_counter)
      {
        NativeCPC->lightgun_counter--;
      }

      flags |= HDT_flag; // enable horizontal display
    } /* if (flags & HT_flag) */

    // ----------------------------------------------------------------

    if (CRTC->char_count >= CRTC->registers[0]) // cc = horizontal total?
    {
      flags |= HT_flag; // takes effect on next cc
    }

    if (CRTC->char_count == CRTC->registers[1]) // cc = horizontal displayed?
    {
      flags &= ~HDT_flag; // disable horizontal display
      CRTC->last_hdisp = CRTC->char_count; // save width for line advancement
    }

    // check hsw ------------------------------------------------------

    if (flags & HS_flag) // in horizontal sync?
    {
      if (VDU->hdelay == 2) // ready to trigger VDU HSYNC?
      {
        if (--VDU->hsw_count == 0)
        {
          if (VDU->scr_line++ < CPC_SCR_HEIGHT)
          {
#ifdef NO_BORDER_RENDER
            if ((VDU->vcount) && (VDU->scanline > VDU_BORDERSHIFT)) // in the visible portion of the screen, no border render?
#else
            if (VDU->vcount)  // in the visible portion of the screen?
#endif
            {
              VDU->scr_base += VDU->scr_line_offs; // advance to next line
            }
          }

          VDU->scr_current = VDU->scr_base;
          VDU->char_count = 0;

          VDU->hdelay++; // prevent reentry
        }
      }
      else
      {
        VDU->hdelay++; // update delay counter
      }

      // hsw end -----------------------------------------------------

      if (--CRTC->hsw_count == 0) // end of HSYNC?
      {
        flags &= ~HS_flag; // reset HSYNC
        CRTC->current_mode = CRTC->requested_mode;
        CRTC->CurrentHandler = CRTC->RequestedHandler;
        VDU->scanline++;

        if (VDU->vdelay) // monitor delaying VSYNC?
        {
          VDU->vdelay--;
        }

        if (VDU->vdelay == 0) // enter monitor VSYNC?
        {
          if (VDU->vsw_count) // still in monitor VSYNC?
          {
            if (--VDU->vsw_count == 0) // done with monitor VSYNC?
            {
              if (VDU->scanline > VDU_SCANLINE_MIN) // above minimum scanline count?
              {
                VDU->scanline = 0;
                VDU->scr_line = 0;
                VDU->scr_current = VDU->scr_base;
                VDU->frame_completed = 1; // force exit of emulation loop
              }
            }
          }
        }

        // GA interrupt trigger -------------------------------------

        GateArray->sl_count++; // update GA scanline counter
        if (GateArray->int_delay) // delay on VSYNC?
        {
          if (--GateArray->int_delay == 0) // delay expired?
          {
            if (GateArray->sl_count >= 32) // counter above save margin?
            {
              NativeCPC->Z80->Regs.int_pending = 1; // queue interrupt
            }

            GateArray->sl_count = 0; // clear counter
          }
        }

        if (GateArray->sl_count == 52) // trigger interrupt?
        {
          NativeCPC->Z80->Regs.int_pending = 1; // queue interrupt
          GateArray->sl_count = 0; // clear counter
        }
      }
    } /* if (flags & HS_flag) */

    // ----------------------------------------------------------------

    if (CRTC->char_count == CRTC->registers[2]) // cc = horizontal sync position?
    {
      if (CRTC->hsw_active) // allow HSYNCs?
      {
        flags |= HS_flag; // set HSYNC
        CRTC->hsw_count = CRTC->hsw_active; // load hsw counter
        VDU->hdelay = 0; // clear VDU 2 chars HSYNC delay
        VDU->hsw_count = VDU->hsw_active; // load VDU hsw counter
      }
    }

    if (VDU->hcount)
    {
      if (!CRTC->stop_rendering)
      {
        {
          if ((flags & (HDT_flag | VDT_flag)) == (HDT_flag | VDT_flag)) // DISPTMG enabled?
          {
            if (CRTC->skew)
            {
              CRTC->skew--;
              if (VDU->scanline > VDU_BORDERSHIFT - 1)
              {
                if (VDU->char_count > 11)
                {
                  VIDEO_DRAW_BORDER;
                }
              }
            }
            else
            {
              tULong addr = (CRTC->char_count * 2) + CRTC->addr; // cc x2 + CPC screen memory address
              if (addr & 0x2000) // 32K screen?
              {
                addr += 0x4000; // advance to next 16K segment
              }
              addr &= 0xC7FF; // apply 11000111 mask to MSB (bits 15-8)
              addr |= (CRTC->raster_count & 0x38) << 8; // insert rc, masked with 00111000

              // call apropriate mode handler
              if (VDU->scanline > VDU_BORDERSHIFT - 1)
              {
                if (VDU->char_count > 11)
                {
        					PROFILE_ADD_NATIVE(PROFILE_video_draw);
                  CRTC->CurrentHandler(VDU,
                                       addr);
                }
              }
            }
          }
          else
          {
            if (VDU->scanline > VDU_BORDERSHIFT - 1)
            {
              if (VDU->char_count > 11)
              {
                VIDEO_DRAW_BORDER;
              }
            }
          }
        }
      }

      VDU->hcount--;
    }
    else /* if (VDU->hcount) */
    {
      if (VDU->char_count == VDU_HSTART)
      {
        if (VDU->vcount)
        {
          VDU->hcount = VDU_HWIDTH;
          VDU->vcount--;
        }
        else if (VDU->scr_line == VDU_VSTART)
        {
          VDU->vcount = VDU_VHEIGHT;
        }
      }
    } /* if (VDU->hcount) */

    if (NativeCPC->lightgun_beam_detect)
    {
      if ( (NativeCPC->lightgun_y_pos == CRTC->line_count) &&
           (NativeCPC->lightgun_x_pos == CRTC->char_count) )
      {
        // Check if drawn pixel is bright enough.
#if defined(__PALMOS__)
          if (NativeCPC->PalmPaletteIntensity[(tUChar)*(VDU->scr_current-1)] >= (tUChar)NativeCPC->lightgun_sensitivity)
#else /* __PALMOS__ */
          if (NativeCPC->WinPalettePtr[(tUChar)*(VDU->scr_current-1)].intensity >= (tUChar)NativeCPC->lightgun_sensitivity)
#endif /* __PALMOS__ */
        {
#ifdef _DEBUG
          NativeCPC->lightgun_debug_flags |= 1;
#endif /* _DEBUG */
          NativeCPC->lightgun_counter = LIGHTGUN_RASTER_INIT;
        }
      }
    }
  }
  while (--repeat_count);

  CRTC->flags = flags;
}
/*---------------------------------------------------------------------*/


static tVoid video_draw_mode0(tVDU* VDU,
                              tULong addr)
/***********************************************************************
 *
 *  video_draw_mode0
 *
 ***********************************************************************/
{
tULong Pixels1, Pixels2;
tULong* Palette_ptr;
tUShort ModeTable1, ModeTable2;
tUShort RAM;

  Palette_ptr = VDU->palette;
  RAM = *((tUShort*)(VDU->pbRAM + addr));

  ModeTable1 = *((tUShort*)(VDU->mode0_table + ((RAM & 0xff) * 2)));
  ModeTable2 = *((tUShort*)(VDU->mode0_table + ((RAM & 0xff00) >> 7)));

  Pixels1  = *(Palette_ptr + ((ModeTable1 >> 8) & 0xff)) << 16;
  Pixels1 |= *(Palette_ptr + (ModeTable1 & 0xff)) & 0xff;
  Pixels1 |= Pixels1 << 8;

  Pixels2  = *(Palette_ptr + ((ModeTable2 >> 8) & 0xff)) << 16;
  Pixels2 |= *(Palette_ptr + (ModeTable2 & 0xff)) & 0xff;
  Pixels2 |= Pixels2 << 8;

  // 8 pixels transfer to screen memory
  *(VDU->scr_current++) = Pixels1; // write four pixels
  *(VDU->scr_current++) = Pixels2; // write four pixels
}
/*---------------------------------------------------------------------*/


static tVoid video_draw_mode1(tVDU* VDU,
                              tULong addr)
/***********************************************************************
 *
 *  video_draw_mode1
 *
 ***********************************************************************/
{
tULong Pixels1, Pixels2;
tULong* Palette_ptr;
tULong ModeTable1;
tULong ModeTable2;
tUShort RAM;

  Palette_ptr = VDU->palette;

  RAM = *((tUShort*)(VDU->pbRAM + addr));
  ModeTable1 = *(tULong*)(VDU->mode1_table + ((RAM & 0xff) * 4));
  ModeTable2 = *(tULong*)(VDU->mode1_table + ((RAM & 0xff00) >> 6));

  Pixels1  = *(Palette_ptr + (ModeTable1 & 0xff));
  Pixels1 |= *(Palette_ptr + ((ModeTable1 >>  8) & 0xff)) << 8;
  Pixels1 |= *(Palette_ptr + ((ModeTable1 >> 16) & 0xff)) << 16;
  Pixels1 |= *(Palette_ptr + ((ModeTable1 >> 24) & 0xff)) << 24;

  Pixels2  = *(Palette_ptr + (ModeTable2 & 0xff));
  Pixels2 |= *(Palette_ptr + ((ModeTable2 >>  8) & 0xff)) << 8;
  Pixels2 |= *(Palette_ptr + ((ModeTable2 >> 16) & 0xff)) << 16;
  Pixels2 |= *(Palette_ptr + ((ModeTable2 >> 24) & 0xff)) << 24;

  // 8 pixels transfer to screen memory
  *(VDU->scr_current++) = Pixels1; // write four pixels
  *(VDU->scr_current++) = Pixels2; // write four pixels
}
/*---------------------------------------------------------------------*/


#define VIDEO_DRAW_MODE_2 \
{ \
  val1  = (RAM & 0x80 ? pen_on : pen_off); \
  val1 |= (RAM & 0x20 ? pen_on : pen_off) << 8; \
  val1 |= (RAM & 0x08 ? pen_on : pen_off) << 16; \
  val1 |= (RAM & 0x02 ? pen_on : pen_off) << 24; \
\
  val2  = (RAM & 0x8000 ? pen_on : pen_off); \
  val2 |= (RAM & 0x2000 ? pen_on : pen_off) << 8; \
  val2 |= (RAM & 0x0800 ? pen_on : pen_off) << 16; \
  val2 |= (RAM & 0x0200 ? pen_on : pen_off) << 24; \
}


static tVoid video_draw_mode2(tVDU* VDU,
                              tULong addr)
/***********************************************************************
 *
 *  video_draw_mode2
 *
 ***********************************************************************/
{
tULong pen_off = VDU->palette[0];
tULong pen_on = VDU->palette[1];
tULong val1, val2;
tUShort RAM = *((tUShort*)(VDU->pbRAM + addr));

  VIDEO_DRAW_MODE_2;

  // 8 pixels transfer to screen memory
  *(VDU->scr_current++) = val1; // write four pixels
  *(VDU->scr_current++) = val2; // write four pixels
}
/*---------------------------------------------------------------------*/


static tVoid video_draw_mode2_antialiased(tVDU* VDU,
                                          tULong addr)
/***********************************************************************
 *
 *  video_draw_mode2_antialiased
 *
 ***********************************************************************/
{
tULong pen_off = VDU->palette[0];
tULong pen_on = VDU->palette[1];
tULong pen_half = VDU->palette[2];
tULong val1, val2;
tUShort RAM = *((tUShort*)(VDU->pbRAM + addr));

  if (pen_half == pen_off)
  {
    VIDEO_DRAW_MODE_2;
  }
  else
  {
    tULong Mask = RAM & 0xC0;
    val1  = (Mask == 0 ? pen_off : Mask == 0xC0 ? pen_on : pen_half);
    Mask = RAM & 0x30;
    val1 |= (Mask == 0 ? pen_off : Mask == 0x30 ? pen_on : pen_half) << 8;
    Mask = RAM & 0x0C;
    val1 |= (Mask == 0 ? pen_off : Mask == 0x0C ? pen_on : pen_half) << 16;
    Mask = RAM & 0x03;
    val1 |= (Mask == 0 ? pen_off : Mask == 0x03 ? pen_on : pen_half) << 24;

    Mask = RAM & 0xC000;
    val2  = (Mask == 0 ? pen_off : Mask == 0xC000 ? pen_on : pen_half);
    Mask = RAM & 0x3000;
    val2 |= (Mask == 0 ? pen_off : Mask == 0x3000 ? pen_on : pen_half) << 8;
    Mask = RAM & 0x0C00;
    val2 |= (Mask == 0 ? pen_off : Mask == 0x0C00 ? pen_on : pen_half) << 16;
    Mask = RAM & 0x0300;
    val2 |= (Mask == 0 ? pen_off : Mask == 0x0300 ? pen_on : pen_half) << 24;
  }

  // 8 pixels transfer to screen memory
  *(VDU->scr_current++) = val1; // write four pixels
  *(VDU->scr_current++) = val2; // write four pixels
}
/*---------------------------------------------------------------------*/


//
// AUDIO
//

static tVoid audio_init(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_init
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tULong n;

  for (n = 0; n < 16; n++)
  {
    audio_set_AY_Register(PSG,
                          n,
                          PSG->RegisterAY.Index[n]); // init sound emulation with valid values
  }

  AUDIO_SET_CASE_ENV(PSG->RegisterAY.UChar.EnvType);
}
/*----------------------------------------------------------------------------*/


static inline tVoid audio_set_AY_Register(tPSG* PSG,
                                          tULong Num,
                                          tULong Value)
/***********************************************************************
 *
 *  audio_set_AY_Register
 *
 ***********************************************************************/
{
  switch (Num)
  {
    case 13:
      audio_set_envelope_register(PSG, Value & 15);
      break;

    case 1:
    case 3:
    case 5:
      PSG->RegisterAY.Index[Num] = Value & 15;
      break;

    case 6:
      PSG->RegisterAY.UChar.Noise = Value & 31;
      break;

    case 7:
      PSG->RegisterAY.UChar.Mixer = Value & 63;
      PSG->Ton_EnA = Value & 1 ? cFalse : cTrue;
      PSG->Noise_EnA = Value & 8 ? cFalse : cTrue;
      PSG->Ton_EnB = Value & 2 ? cFalse : cTrue;
      PSG->Noise_EnB = Value & 16 ? cFalse : cTrue;
      PSG->Ton_EnC = Value & 4 ? cFalse : cTrue;
      PSG->Noise_EnC = Value & 32 ? cFalse : cTrue;
      break;

    case 8:
      PSG->RegisterAY.UChar.AmplitudeA = Value & 31;
      PSG->Envelope_EnA = Value & 16 ? cFalse : cTrue;
      break;

    case 9:
      PSG->RegisterAY.UChar.AmplitudeB = Value & 31;
      PSG->Envelope_EnB = Value & 16 ? cFalse : cTrue;
      break;

    case 10:
      PSG->RegisterAY.UChar.AmplitudeC = Value & 31;
      PSG->Envelope_EnC = Value & 16 ? cFalse : cTrue;
      break;

    case 0:
    case 2:
    case 4:
    case 11:
    case 12:
      PSG->RegisterAY.Index[Num] = Value;
      break;
  }
}
/*----------------------------------------------------------------------------*/


static inline tVoid audio_set_envelope_register(tPSG* PSG,
                                                tULong Value)
/***********************************************************************
 *
 *  audio_set_envelope_register
 *
 ***********************************************************************/
{
  PSG->Envelope_Counter = 0;
  PSG->FirstPeriod = cTrue;
  if (!(Value & 4))
  {
    PSG->AmplitudeEnv = 32;
  }
  else
  {
    PSG->AmplitudeEnv = -1;
  }

  PSG->RegisterAY.UChar.EnvType = Value;
  AUDIO_SET_CASE_ENV(Value);
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_0_3__9(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_0_3__9
 *
 ***********************************************************************/
{
  if (PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv--;
    if (!PSG->AmplitudeEnv)
    {
      PSG->FirstPeriod = cFalse;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_4_7__15(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_4_7__15
 *
 ***********************************************************************/
{
  if (PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv++;
    if (PSG->AmplitudeEnv == 32)
    {
      PSG->FirstPeriod = cFalse;
      PSG->AmplitudeEnv = 0;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_8(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_8
 *
 ***********************************************************************/
{
  PSG->AmplitudeEnv = (PSG->AmplitudeEnv - 1) & 31;
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_10(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_10
 *
 ***********************************************************************/
{
  if (PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv--;
    if (PSG->AmplitudeEnv == -1)
    {
      PSG->FirstPeriod = cFalse;
      PSG->AmplitudeEnv = 0;
    }
  }
  else
  {
    PSG->AmplitudeEnv++;
    if (PSG->AmplitudeEnv == 32)
    {
      PSG->FirstPeriod = cTrue;
      PSG->AmplitudeEnv = 31;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_11(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_11
 *
 ***********************************************************************/
{
  if (PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv--;
    if (PSG->AmplitudeEnv == -1)
    {
      PSG->FirstPeriod = cFalse;
      PSG->AmplitudeEnv = 31;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_12(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_12
 *
 ***********************************************************************/
{
  PSG->AmplitudeEnv = (PSG->AmplitudeEnv + 1) & 31;
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_13(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_13
 *
 ***********************************************************************/
{
  if (PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv++;
    if (PSG->AmplitudeEnv == 32)
    {
      PSG->FirstPeriod = cFalse;
      PSG->AmplitudeEnv = 31;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid audio_Case_EnvType_14(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Case_EnvType_14
 *
 ***********************************************************************/
{
  if (!PSG->FirstPeriod)
  {
    PSG->AmplitudeEnv--;
    if (PSG->AmplitudeEnv == -1)
    {
      PSG->FirstPeriod = cTrue;
      PSG->AmplitudeEnv = 0;
    }
  }
  else
  {
    PSG->AmplitudeEnv++;
    if (PSG->AmplitudeEnv == 32)
    {
      PSG->FirstPeriod = cFalse;
      PSG->AmplitudeEnv = 31;
    }
  }
}
/*----------------------------------------------------------------------------*/


static inline tVoid audio_Synthesizer_Logic_Q(tPSG* PSG)
/***********************************************************************
 *
 *  audio_Synthesizer_Logic_Q
 *
 ***********************************************************************/
{
  PSG->Ton_Counter_A++;
  if (PSG->Ton_Counter_A >= *(tUShort*)&PSG->RegisterAY.UChar.TonALo) // Little Endian ONLY !!
  {
    PSG->Ton_Counter_A = 0;
    PSG->Ton_A ^= 1;
  }

  PSG->Ton_Counter_B++;
  if (PSG->Ton_Counter_B >= *(tUShort*)&PSG->RegisterAY.UChar.TonBLo) // Little Endian ONLY !!
  {
    PSG->Ton_Counter_B = 0;
    PSG->Ton_B ^= 1;
  }

  PSG->Ton_Counter_C++;
  if (PSG->Ton_Counter_C >= *(tUShort*)&PSG->RegisterAY.UChar.TonCLo) // Little Endian ONLY !!
  {
    PSG->Ton_Counter_C = 0;
    PSG->Ton_C ^= 1;
  }

  PSG->Noise_Counter++;
  if ( (!(PSG->Noise_Counter & 1)) && (PSG->Noise_Counter >= (PSG->RegisterAY.UChar.Noise << 1)) )
  {
    PSG->Noise_Counter = 0;
    PSG->Noise.Seed = (((((PSG->Noise.Seed >> 13) ^ (PSG->Noise.Seed >> 16)) & 1) ^ 1) | PSG->Noise.Seed << 1) & 0x1ffff;
  }

  if (!PSG->Envelope_Counter)
  {
    PSG->Case_Env(PSG);
  }

  PSG->Envelope_Counter++;

  if ( PSG->Envelope_Counter >=
       ((PSG->RegisterAY.UChar.EnvelopeHi << 8) + PSG->RegisterAY.UChar.EnvelopeLo) )
  //FC!!if ( PSG->Envelope_Counter >= *(tUShort*)&PSG->RegisterAY.UChar.EnvelopeLo) // Little Endian ONLY !!
  {
    PSG->Envelope_Counter = 0;
  }
}
/*----------------------------------------------------------------------------*/


#undef ENABLE_AUDIO_CHANNEL_A
#undef ENABLE_AUDIO_CHANNEL_B
#undef ENABLE_AUDIO_CHANNEL_C
#define ENABLE_AUDIO_CHANNEL_A
#define ENABLE_AUDIO_CHANNEL_B
#define ENABLE_AUDIO_CHANNEL_C

#if SND_STEREO == 1

static tVoid inline audio_Synthesizer_Mixer_Q(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_Synthesizer_Mixer_Q
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tLong LevL;
tLong LevR;
tLong k;

#ifdef ENABLE_TAPE
  LevL = PSG->bTapeLevel ? PSG->LevelTape : 0; // start with the tape signal
#else /* ENABLE_TAPE */
  LevL = 0;
#endif /* ENABLE_TAPE */

#ifdef ENABLE_PRINTER
  if (PSG->snd_pp_device)
  {
    LevL += PSG->Level_PP[NativeCPC->printer_port];
  }
#endif /* ENABLE_PRINTER */

  LevR = LevL;

#ifdef ENABLE_AUDIO_CHANNEL_A

  //
  // Channel A
  //
  if (PSG->Ton_EnA)
  {
    if ( (!PSG->Envelope_EnA) ||
         (*(tUShort*)&PSG->RegisterAY.UChar.TonALo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_A;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnA)
  {
    k &= PSG->Noise.s.Val;
  }

  if (k)
  {
    if (PSG->Envelope_EnA)
    {
      LevL += PSG->Level_AL[PSG->RegisterAY.UChar.AmplitudeA * 2 + 1];
      LevR += PSG->Level_AR[PSG->RegisterAY.UChar.AmplitudeA * 2 + 1];
    }
    else
    {
      LevL += PSG->Level_AL[PSG->AmplitudeEnv];
      LevR += PSG->Level_AR[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_A */

#ifdef ENABLE_AUDIO_CHANNEL_B

  //
  // Channel B
  //
  if (PSG->Ton_EnB)
  {
    if ( (!PSG->Envelope_EnB) ||
         (*(tUShort*)&PSG->RegisterAY.UChar.TonBLo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_B;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnB)
  {
    k &= PSG->Noise.s.Val;
  }
  if (k)
  {
    if (PSG->Envelope_EnB)
    {
      LevL += PSG->Level_BL[PSG->RegisterAY.UChar.AmplitudeB * 2 + 1];
      LevR += PSG->Level_BR[PSG->RegisterAY.UChar.AmplitudeB * 2 + 1];
    }
    else
    {
      LevL += PSG->Level_BL[PSG->AmplitudeEnv];
      LevR += PSG->Level_BR[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_B */

#ifdef ENABLE_AUDIO_CHANNEL_C

  //
  // Channel C
  //
  if (PSG->Ton_EnC)
  {
    if ( (!PSG->Envelope_EnC) ||
         (*(tUShort*)&PSG->RegisterAY.UChar.TonCLo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_C;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnC)
  {
    k &= PSG->Noise.s.Val;
  }
  if (k)
  {
    if (PSG->Envelope_EnC)
    {
      LevL += PSG->Level_CL[PSG->RegisterAY.UChar.AmplitudeC * 2 + 1];
      LevR += PSG->Level_CR[PSG->RegisterAY.UChar.AmplitudeC * 2 + 1];
    }
    else
    {
      LevL += PSG->Level_CL[PSG->AmplitudeEnv];
      LevR += PSG->Level_CR[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_C */

  PSG->Left_Chan += LevL;
  PSG->Right_Chan += LevR;
}
/*----------------------------------------------------------------------------*/


#undef AUDIO_DISABLE_LEFT
//#define AUDIO_DISABLE_LEFT
#undef AUDIO_DISABLE_RIGHT
//#define AUDIO_DISABLE_RIGHT

#if SND_16BITS == 1

static tVoid ATTR_EXTRA audio_Synthesizer_Stereo16(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_Synthesizer_Stereo16
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tULong Tick_Counter = PSG->LoopCount >> 16;
tNativeULongField SampleValue;

  PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Stereo16);

  while (PSG->LoopCount >= 65536)
  {
    PSG->LoopCount -= 65536;

    audio_Synthesizer_Logic_Q(PSG);
    audio_Synthesizer_Mixer_Q(NativeCPC);

    PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Stereo16_Loop);
  }

  PSG->LoopCount += SND_LOOP_COUNT_INIT;

#ifndef AUDIO_DISABLE_LEFT
  SampleValue.UShort.usL = SND_SAMPLE_OFFSET_16BIT + PSG->Left_Chan / Tick_Counter;
#else /* AUDIO_DISABLE_LEFT */
  SampleValue.UShort.usL = 0;
#endif /* AUDIO_DISABLE_LEFT */

#ifndef AUDIO_DISABLE_RIGHT
  SampleValue.UShort.usH = SND_SAMPLE_OFFSET_16BIT + PSG->Right_Chan / Tick_Counter;
#else /* AUDIO_DISABLE_RIGHT */
  SampleValue.UShort.usH = 0;
#endif /* AUDIO_DISABLE_RIGHT */

  if (PSG->FilledBufferSize < SND_BUFFER_SIZE)
  {
    *(tULong*)PSG->snd_bufferptr = SampleValue.ULong; // write to mixing buffer
    PSG->snd_bufferptr += 4;
    PSG->FilledBufferSize += 4;
  }

  PSG->Left_Chan = 0;
  PSG->Right_Chan = 0;

  if (PSG->snd_bufferptr >= PSG->pbSndBufferEnd)
  {
    PSG->snd_bufferptr = PSG->pbSndBuffer;
    PSG->buffer_full = 1;
  }
}
/*----------------------------------------------------------------------------*/

#else /* SND_16BITS */

static tVoid ATTR_EXTRA audio_Synthesizer_Stereo8(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_Synthesizer_Stereo8
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tULong Tick_Counter = PSG->LoopCount >> 16;
tNativeUShortField SampleValue;

  PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Stereo8);

  while (PSG->LoopCount >= 65536)
  {
    PSG->LoopCount -= 65536;

    audio_Synthesizer_Logic_Q(PSG);
    audio_Synthesizer_Mixer_Q(NativeCPC);

    PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Stereo8_Loop);
  }

  PSG->LoopCount += SND_LOOP_COUNT_INIT;

#ifndef AUDIO_DISABLE_LEFT
  SampleValue.UChar.ucL = SND_SAMPLE_OFFSET_8BIT + PSG->Left_Chan / Tick_Counter;
#else /* AUDIO_DISABLE_LEFT */
  SampleValue.UChar.ucL = 0;
#endif /* AUDIO_DISABLE_LEFT */

#ifndef AUDIO_DISABLE_RIGHT
  SampleValue.UChar.ucH = SND_SAMPLE_OFFSET_8BIT + PSG->Right_Chan / Tick_Counter;
#else /* AUDIO_DISABLE_RIGHT */
  SampleValue.UChar.ucH = 0;
#endif /* AUDIO_DISABLE_RIGHT */

  if (PSG->FilledBufferSize < SND_BUFFER_SIZE)
  {
    *(tUShort*)PSG->snd_bufferptr = SampleValue.UShort; // write to mixing buffer
    PSG->snd_bufferptr += 2;
    PSG->FilledBufferSize += 2;
  }

  PSG->Left_Chan = 0;
  PSG->Right_Chan = 0;

  if (PSG->snd_bufferptr >= PSG->pbSndBufferEnd)
  {
    PSG->snd_bufferptr = PSG->pbSndBuffer;
    PSG->buffer_full = 1;
  }
}
/*----------------------------------------------------------------------------*/

#endif /* SND_16BITS */

#else /* SND_STEREO == 1 */

static inline tVoid audio_Synthesizer_Mixer_Q_Mono(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_Synthesizer_Mixer_Q_Mono
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tLong Lev;
tLong k;

#ifdef ENABLE_TAPE
  Lev = PSG->bTapeLevel ? PSG->LevelTape : 0; // start with the tape signal
#else /* ENABLE_TAPE */
  Lev = 0;
#endif /* ENABLE_TAPE */

#ifdef ENABLE_PRINTER
  if (PSG->snd_pp_device)
  {
    Lev += PSG->Level_PP[NativeCPC->printer_port];
  }
#endif /* ENABLE_PRINTER */

#ifdef ENABLE_AUDIO_CHANNEL_A

  //
  // Channel A
  //
  if (PSG->Ton_EnA)
  {
    if ((!PSG->Envelope_EnA) || (*(tUShort*)&PSG->RegisterAY.UChar.TonALo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_A;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnA)
  {
    k &= PSG->Noise.s.Val;
  }

  if (k)
  {
    if (PSG->Envelope_EnA)
    {
      Lev += PSG->Level_AL[PSG->RegisterAY.UChar.AmplitudeA * 2 + 1];
    }
    else
    {
      Lev += PSG->Level_AL[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_A */

#ifdef ENABLE_AUDIO_CHANNEL_B

  //
  // Channel B
  //
  if (PSG->Ton_EnB)
  {
    if ((!PSG->Envelope_EnB) || (*(tUShort*)&PSG->RegisterAY.UChar.TonBLo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_B;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnB)
  {
    k &= PSG->Noise.s.Val;
  }
  if (k)
  {
    if (PSG->Envelope_EnB)
    {
      Lev += PSG->Level_BL[PSG->RegisterAY.UChar.AmplitudeB * 2 + 1];
    }
    else
    {
      Lev += PSG->Level_BL[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_B */

#ifdef ENABLE_AUDIO_CHANNEL_C

  //
  // Channel C
  //
  if (PSG->Ton_EnC)
  {
    if ((!PSG->Envelope_EnC) || (*(tUShort*)&PSG->RegisterAY.UChar.TonCLo > 4)) // Little Endian ONLY !!
    {
      k = PSG->Ton_C;
    }
    else
    {
      k = 1;
    }
  }
  else
  {
    k = 1;
  }

  if (PSG->Noise_EnC)
  {
    k &= PSG->Noise.s.Val;
  }
  if (k)
  {
    if (PSG->Envelope_EnC)
    {
      Lev += PSG->Level_CL[PSG->RegisterAY.UChar.AmplitudeC * 2 + 1];
    }
    else
    {
      Lev += PSG->Level_CL[PSG->AmplitudeEnv];
    }
  }

#endif /* ENABLE_AUDIO_CHANNEL_C */

  PSG->Left_Chan += Lev;
}
/*----------------------------------------------------------------------------*/

static tVoid ATTR_EXTRA audio_Synthesizer_Mono8(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  audio_Synthesizer_Mono8
 *
 ***********************************************************************/
{
tPSG* PSG = NativeCPC->PSG;
tULong Tick_Counter = PSG->LoopCount >> 16;
tUChar SampleValue;

  PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Mono8);

  while (PSG->LoopCount >= 65536)
  {
    PSG->LoopCount -= 65536;

    audio_Synthesizer_Logic_Q(PSG);
    audio_Synthesizer_Mixer_Q_Mono(NativeCPC);

    PROFILE_ADD_NATIVE(PROFILE_audio_Synthesizer_Mono8_Loop);
  }

  PSG->LoopCount += SND_LOOP_COUNT_INIT;

  SampleValue = SND_SAMPLE_OFFSET_8BIT + PSG->Left_Chan / Tick_Counter;

  if (PSG->FilledBufferSize < SND_BUFFER_SIZE)
  {
    *PSG->snd_bufferptr = SampleValue; // write to mixing buffer
    PSG->snd_bufferptr++;
    PSG->FilledBufferSize++;
  }

  PSG->Left_Chan = 0;

  if (PSG->snd_bufferptr >= PSG->pbSndBufferEnd)
  {
    PSG->snd_bufferptr = PSG->pbSndBuffer;
    PSG->buffer_full = 1;
  }
}
/*----------------------------------------------------------------------------*/

#endif /* SND_STEREO == 1 */


//
// FDC
//

static tVoid fdc_init(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_init
 *
 ***********************************************************************/
{
tFDC* FDC = NativeCPC->FDC;
fdc_cmd_table_def* TableIndex = FDC->FDCCommandTable;

  /* Cmd=0x03 */ TableIndex[0].cmd_handler  = (tCmdHandlerPtr)fdc_specify;
  /* Cmd=0x04 */ TableIndex[1].cmd_handler  = (tCmdHandlerPtr)fdc_drvstat;
  /* Cmd=0x07 */ TableIndex[2].cmd_handler  = (tCmdHandlerPtr)fdc_recalib;
  /* Cmd=0x08 */ TableIndex[3].cmd_handler  = (tCmdHandlerPtr)fdc_intstat;
  /* Cmd=0x0f */ TableIndex[4].cmd_handler  = (tCmdHandlerPtr)fdc_seek;
  /* Cmd=0x42 */ TableIndex[5].cmd_handler  = (tCmdHandlerPtr)fdc_readtrk;
  /* Cmd=0x45 */ TableIndex[6].cmd_handler  = (tCmdHandlerPtr)fdc_write;
  /* Cmd=0x46 */ TableIndex[7].cmd_handler  = (tCmdHandlerPtr)fdc_read;
  /* Cmd=0x49 */ TableIndex[8].cmd_handler  = (tCmdHandlerPtr)fdc_write;
  /* Cmd=0x4a */ TableIndex[9].cmd_handler  = (tCmdHandlerPtr)fdc_readID;
  /* Cmd=0x4c */ TableIndex[10].cmd_handler = (tCmdHandlerPtr)fdc_read;
  /* Cmd=0x4d */ TableIndex[11].cmd_handler = (tCmdHandlerPtr)fdc_writeID;
  /* Cmd=0x51 */ TableIndex[12].cmd_handler = (tCmdHandlerPtr)fdc_scan;
  /* Cmd=0x59 */ TableIndex[13].cmd_handler = (tCmdHandlerPtr)fdc_scan;
  /* Cmd=0x5d */ TableIndex[14].cmd_handler = (tCmdHandlerPtr)fdc_scan;


  // for (int i=0;i<15;i++)
  // {
  //   printf("%p %4x %4x %4x %4x\n",TableIndex[i].cmd_handler,TableIndex[i].cmd,
  //     TableIndex[i].cmd_length,TableIndex[i].res_length, TableIndex[i].cmd_direction);
  // }

#ifdef _DEBUG
  FDC->debug_current_operation_index = (tULong)-1;
#endif /* _DEBUG */
}
/*----------------------------------------------------------------------------*/


#define FDC_CHECK_UNIT \
{ \
  FDC->active_drive = FDC->command[CMD_UNIT] & 1 ? NativeCPC->DriveB : NativeCPC->DriveA; \
}


static tULong fdc_read_status(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_read_status
 *
 ***********************************************************************/
{
tULong val;

  val = 0x80; // data register ready

  if (FDC->phase == EXEC_PHASE) // in execution phase?
  {
    if (FDC->read_status_delay)
    {
      val = 0x10; // FDC is busy
      FDC->read_status_delay--;
    }
    else
    {
      val |= 0x30; // FDC is executing & busy
    }

    if (FDC->cmd_direction == FDC_TO_CPU)
    {
      val |= 0x40; // FDC is sending data to the CPU
    }
  }

  else if (FDC->phase == RESULT_PHASE) // in result phase?
  {
    val |= 0x50; // FDC is sending data to the CPU, and is busy
  }

  else // in command phase
  {
    if (FDC->byte_count) // receiving command parameters?
    {
      val |= 0x10; // FDC is busy
    }
  }

  return val;
}
/*----------------------------------------------------------------------------*/


static tULong fdc_read_data(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_read_data
 *
 ***********************************************************************/
{
tULong val;

  val = 0xff; // default value

  //
  // in execution phase?
  //
  if (FDC->phase == EXEC_PHASE)
  {
    if (FDC->cmd_direction == FDC_TO_CPU) // proper direction?
    {
      FDC->timeout = OVERRUN_TIMEOUT;
      val = *FDC->buffer_ptr++; // read byte from current sector

      if (FDC->buffer_ptr >= FDC->buffer_endptr)
      {
        FDC->buffer_ptr = FDC->active_track->data; // wrap around
      }

      // Original
      //if (!(--FDC->buffer_count)) // completed the data transfer?
      if (FDC->buffer_count) FDC->buffer_count--;

      if (!FDC->buffer_count) // completed the data transfer?
      {
        if (FDC->flags & RNDDE_flag) // simulate random Data Errors?
        {
          // ***! random DE handling
        }

        FDC->active_drive->current_sector++; // increase sector index

        if (FDC->flags & OVERRUN_flag) // overrun condition detected?
        {
          FDC->flags &= ~OVERRUN_flag;
          FDC->result[RES_ST0] |= 0x40; // AT
          FDC->result[RES_ST1] |= 0x10; // Overrun

          LOAD_RESULT_WITH_CHRN;

          FDC->phase = RESULT_PHASE; // switch to result phase
        }
        else
        {
          if (FDC->command[CMD_CODE] == 0x42) // read track command?
          {
            if ((--FDC->command[CMD_EOT])) // continue reading sectors?
            {
              if (FDC->active_drive->current_sector >= FDC->active_track->sectors) // index beyond number of sectors for this track?
              {
                FDC->active_drive->current_sector = 0; // reset index
              }

              FDC->command[CMD_R]++; // advance to next sector

              fdc_cmd_readtrk(FDC);
            }
            else
            {
              LOAD_RESULT_WITH_STATUS;
              LOAD_RESULT_WITH_CHRN;

              FDC->phase = RESULT_PHASE; // switch to result phase
            }
          }
          else // normal read (deleted) data command
          {
            if (!((FDC->result[RES_ST1] & 0x31) || (FDC->result[RES_ST2] & 0x21))) // no error bits set?
            {
              if (FDC->command[CMD_R] != FDC->command[CMD_EOT]) // haven't reached End of Track?
              {
                FDC->command[CMD_R]++; // advance to next sector

                fdc_cmd_read(FDC);
              }
              else
              {
                LOAD_RESULT_WITH_STATUS;
                LOAD_RESULT_WITH_CHRN;

                FDC->phase = RESULT_PHASE; // switch to result phase
              }
            }
            else
            {
              LOAD_RESULT_WITH_STATUS;
              LOAD_RESULT_WITH_CHRN;

              FDC->phase = RESULT_PHASE; // switch to result phase
            }
          }
        }
      }
    }
  }

  //
  // in result phase?
  //
  else if (FDC->phase == RESULT_PHASE)
  {
    val = FDC->result[FDC->byte_count++]; // copy value from buffer

    if (FDC->byte_count == FDC->res_length) // sent all result bytes?
    {
      FDC->flags &= ~SCAN_flag; // reset scan command flag
      FDC->byte_count = 0; // clear byte counter
      FDC->phase = CMD_PHASE; // switch to command phase
      FDC->led = 0; // turn the drive LED off

#ifdef _DEBUG
      {
        tULong debug_loop;

        // Save result.
        for (debug_loop = 0; debug_loop < FDC->res_length; debug_loop++)
        {
          FDC->debug_lastoperations[FDC->debug_current_operation_index][FDC_COMMAND_NBELEMENT+debug_loop] = FDC->result[debug_loop];
        }
      }
#endif /* _DEBUG */
    }
  }

  return val;
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_write_data(tNativeCPC* NativeCPC,
                            tULong val)
/***********************************************************************
 *
 *  fdc_write_data
 *
 ***********************************************************************/
#define FDC_WRITE_DATA_ADD_NEW_OPERATION \
{ \
  tULong debug_loop; \
  \
  /* Set index for new operation. */ \
  FDC->debug_nb_operations++; \
  FDC->debug_current_operation_index++; \
  if (FDC->debug_current_operation_index == FDC_DEBUG_NB_LAST_COMMANDS) \
  { \
    FDC->debug_current_operation_index = 0; \
  } \
  /* Save command. */ \
  FDC->debug_size_commands[FDC->debug_current_operation_index] = FDC->cmd_length; \
  FDC->debug_size_results[FDC->debug_current_operation_index] = FDC->res_length; \
  for (debug_loop = 0; debug_loop < FDC->cmd_length; debug_loop++) \
  { \
    FDC->debug_lastoperations[FDC->debug_current_operation_index][debug_loop] = FDC->command[debug_loop]; \
  } \
}
{
tFDC* FDC = NativeCPC->FDC;
tUChar* pbPtr;
tUChar* pbDataPtr;
tLong sector;
tULong idx;
tULong sector_size;
tULong track_size;

  //
  // in command phase?
  //
  if (FDC->phase == CMD_PHASE)
  {
    if (FDC->byte_count) // receiving command parameters?
    {
      FDC->command[FDC->byte_count++] = val; // copy to buffer

      if (FDC->byte_count == FDC->cmd_length) // received all command bytes?
      {
        FDC->byte_count = 0; // clear byte counter
        FDC->phase = EXEC_PHASE; // switch to execution phase

        FDC->cmd_handler(FDC,
                         NativeCPC);
#ifdef _DEBUG
        FDC_WRITE_DATA_ADD_NEW_OPERATION;
#endif /* _DEBUG */
      }
    }
    else // first command byte received
    {
      if (val & 0x20) // skip DAM or DDAM?
      {
        FDC->flags |= SKIP_flag; // DAM/DDAM will be skipped
        val &= ~0x20; // reset skip bit in command byte
      }
      else
      {
        FDC->flags &= ~SKIP_flag; // make sure skip indicator is off
      }

      for (idx = 0; idx < MAX_CMD_COUNT; idx++) // loop through all known FDC commands
      {
        if (FDC->FDCCommandTable[idx].cmd == val) // do we have a match?
        {
          break;
        }
      }

      if (idx != MAX_CMD_COUNT) // valid command received
      {
        FDC->cmd_length = FDC->FDCCommandTable[idx].cmd_length; // command length in bytes
        FDC->res_length = FDC->FDCCommandTable[idx].res_length; // result length in bytes
        FDC->cmd_direction = FDC->FDCCommandTable[idx].cmd_direction; // direction is CPU to FDC, or FDC to CPU
        FDC->cmd_handler = FDC->FDCCommandTable[idx].cmd_handler; // pointer to command handler

        FDC->command[FDC->byte_count++] = val; // copy command code to buffer

        if (FDC->byte_count == FDC->cmd_length) // 1 byte command requested ?
        {
          FDC->byte_count = 0; // clear byte counter
          FDC->phase = EXEC_PHASE; // switch to execution phase
          FDC->cmd_handler(FDC,
                           NativeCPC);
#ifdef _DEBUG
          FDC_WRITE_DATA_ADD_NEW_OPERATION;
#endif /* _DEBUG */
        }
      }
      else // unknown command received
      {
        FDC->result[RES_ST0] = 0x80; // indicate invalid command
        FDC->res_length = 1;
        FDC->phase = RESULT_PHASE; // switch to result phase
      }
    }
  }

  //
  // in execution phase?
  //
  else if (FDC->phase == EXEC_PHASE)
  {
    if (FDC->cmd_direction == CPU_TO_FDC) // proper direction?
    {
      FDC->timeout = OVERRUN_TIMEOUT;

      if ((FDC->flags & SCAN_flag)) // processing any of the scan commands?
      {
        if (val != 0xff) // no comparison on CPU byte = 0xff
        {
          //switch ((FDC->command[CMD_CODE] & 0x1f)) // Original
          switch ((FDC->command[CMD_CODE] & 0x5f))
          {
            case 0x51: // scan equal
            {
              if (val != *FDC->buffer_ptr)
              {
                FDC->result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
                FDC->flags |= SCANFAILED_flag;
              }
            }
            break;

            case 0x59: // scan low or equal
            {
              if (val != *FDC->buffer_ptr)
              {
                FDC->result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
              }
              if (val > *FDC->buffer_ptr)
              {
                FDC->flags |= SCANFAILED_flag;
              }
            }
            break;

            case 0x5d: // scan high or equal
            {
              if (val != *FDC->buffer_ptr)
              {
                FDC->result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
              }
              if (val < *FDC->buffer_ptr)
              {
                FDC->flags |= SCANFAILED_flag;
              }
            }
            break;
          }
        }
        FDC->buffer_ptr++; // advance sector data pointer
      }
      else
      {
        *FDC->buffer_ptr++ = val; // write byte to sector
      }

      if (FDC->buffer_ptr > FDC->buffer_endptr)
      {
        FDC->buffer_ptr = FDC->active_track->data; // wrap around
      }

      // Original
      //if (--FDC->buffer_count == 0) // processed all data?
      // Fix bug to load "Top Secret"
      if (FDC->buffer_count) FDC->buffer_count--;

      if (!FDC->buffer_count) // processed all data?
      {
        if ((FDC->flags & SCAN_flag)) // processing any of the scan commands?
        {
          if ( (FDC->flags & SCANFAILED_flag) &&
               (FDC->command[CMD_R] != FDC->command[CMD_EOT]) )
          {
            FDC->command[CMD_R] += FDC->command[CMD_STP]; // advance to next sector

            fdc_cmd_scan(FDC);
          }
          else
          {
            if ((FDC->flags & SCANFAILED_flag))
            {
              FDC->result[RES_ST2] |= 0x04; // Scan Not Satisfied
            }

            LOAD_RESULT_WITH_CHRN;

            FDC->phase = RESULT_PHASE; // switch to result phase
          }
        }
        else if (FDC->command[CMD_CODE] == 0x4d) // write ID command?
        {
          sector_size = 128 << FDC->command[CMD_C]; // determine number of bytes from N value
          track_size = sector_size * FDC->command[CMD_H];

          if (FDC->active_track->sectors != 0) // track is formatted?
          {
            MemSetByte(FDC->active_track->data,
                       track_size,
                       0);
          }

          if (((sector_size + 62 + FDC->command[CMD_R]) * FDC->command[CMD_H]) > FDC_MAX_TRACK_SIZE) // track size exceeds maximum ?
          {
            FDC->active_track->sectors = 0; // 'unformat' track
          }
          else
          {
            FDC->active_track->sectors = FDC->command[CMD_H];

            pbDataPtr = FDC->active_track->data;
            pbPtr = NativeCPC->pbGPBuffer;

            for (sector = 0; sector < FDC->command[CMD_H]; sector++)
            {
              //FC!!*((tULong*)FDC->active_track->sector[sector].CHRN) = *((tULong*)pbPtr); // copy CHRN
              MemMoveByte(FDC->active_track->sector[sector].CHRN,
                          pbPtr,
                          4); // copy CHRN
              FDC->active_track->sector[sector].flags[0] = 0; // clear ST1
              FDC->active_track->sector[sector].flags[1] = 0; // clear ST2

              FDC->active_track->sector[sector].data = pbDataPtr; // store pointer to sector data

              pbDataPtr += sector_size;
              pbPtr += 4;
            }

            MemSetByte(FDC->active_track->data,
                       track_size,
                       FDC->command[CMD_N]); // fill track data with specified byte value
          }

          pbPtr = NativeCPC->pbGPBuffer + ((FDC->command[CMD_H]-1) * 4); // pointer to the last CHRN passed to writeID

          // FDC->result = tULong*, pbPtr = tUChar*
          FDC->result[RES_C] = *pbPtr++;
          FDC->result[RES_H] = *pbPtr++;
          FDC->result[RES_R] = *pbPtr++;
          FDC->result[RES_N] = FDC->command[CMD_C]; // overwrite with the N value from the writeID command

          FDC->active_drive->altered = 1; // indicate that the image has been modified

          FDC->phase = RESULT_PHASE; // switch to result phase
        }
        else if (FDC->command[CMD_R] != FDC->command[CMD_EOT]) // haven't reached End of Track?
        {
          FDC->command[CMD_R]++; // advance to next sector

          fdc_cmd_write(FDC);
        }
        else
        {
          FDC->active_drive->altered = 1; // indicate that the image has been modified

          FDC->result[RES_ST0] |= 0x40; // AT
          FDC->result[RES_ST1] |= 0x80; // End of Cylinder

          LOAD_RESULT_WITH_CHRN;

          FDC->phase = RESULT_PHASE; // switch to result phase
        }
      }
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_specify(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_specify
 *
 ***********************************************************************/
{
  FDC->phase = CMD_PHASE; // switch back to command phase (fdc_specify has no result phase!)
  NOT_USED(NativeCPC);
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_drvstat(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_drvstat
 *
 ***********************************************************************/
{
tULong val;

  FDC_CHECK_UNIT;

  val = FDC->command[CMD_UNIT] & 7; // keep head and unit of command

  if ((FDC->active_drive->write_protected) || (FDC->active_drive->tracks == 0)) // write protected, or disk missing?
  {
    val |= 0x48; // set Write Protect + Two Sided (?)
  }

  if ((FDC->active_drive->tracks) && (FDC->motor))
  {
    val |= 0x20; // set Ready
  }

  if (FDC->active_drive->current_track == 0) // drive head is over track 0?
  {
    val |= 0x10; // set Track 0
  }

  FDC->result[RES_ST0] = val;
  FDC->phase = RESULT_PHASE; // switch to result phase
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_recalib(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_recalib
 *
 ***********************************************************************/
{
  FDC->command[CMD_C] = 0; // seek to track 0

  fdc_seek(FDC, NativeCPC);
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_intstat(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_intstat
 *
 ***********************************************************************/
{
tULong val;

  val = FDC->result[RES_ST0] & 0xf8; // clear Head Address and Unit bits

  if (FDC->flags & SEEKDRVA_flag) // seek completed on drive A?
  {
    val |= 0x20; // set Seek End

    FDC->flags &= ~(SEEKDRVA_flag | STATUSDRVA_flag); // clear seek done and status change flags
    FDC->result[RES_ST0] = val;
    FDC->result[RES_ST1] = NativeCPC->DriveA->current_track;
  }

  else if (FDC->flags & SEEKDRVB_flag) // seek completed on drive B?
  {
    val |= 0x21; // set Seek End

    FDC->flags &= ~(SEEKDRVB_flag | STATUSDRVB_flag); // clear seek done and status change flags
    FDC->result[RES_ST0] = val;
    FDC->result[RES_ST1] = NativeCPC->DriveB->current_track;
  }

  else if (FDC->flags & STATUSDRVA_flag) // has the status of drive A changed?
  {
    val = 0xc0; // status change

    if ((NativeCPC->DriveA->tracks == 0) || (!FDC->motor)) // no DSK in the drive, or drive motor is turned off?
    {
      val |= 0x08; // not ready
    }

    FDC->flags &= ~STATUSDRVA_flag; // clear status change flag
    FDC->result[RES_ST0] = val;
    FDC->result[RES_ST1] = NativeCPC->DriveA->current_track;
  }

  else if (FDC->flags & STATUSDRVB_flag) // has the status of drive B changed?
  {
    val = 0xc1; // status change

    if ((NativeCPC->DriveB->tracks == 0) || (!FDC->motor)) // no DSK in the drive, or drive motor is turned off?
    {
      val |= 0x08; // not ready
    }

    FDC->flags &= ~STATUSDRVB_flag; // clear status change flag
    FDC->result[RES_ST0] = val;
    FDC->result[RES_ST1] = NativeCPC->DriveB->current_track;
  }
  else
  {
    val = 0x80; // Invalid Command

    FDC->result[RES_ST0] = val;
    FDC->res_length = 1;
  }

  FDC->phase = RESULT_PHASE; // switch to result phase
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_seek(tFDC* FDC,
                      tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_seek
 *
 ***********************************************************************/
{
  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_track = FDC->command[CMD_C];

    if (FDC->active_drive->current_track >= DSK_TRACKMAX) // beyond valid range?
    {
      FDC->active_drive->current_track = DSK_TRACKMAX-1; // limit to maximum
    }
  }

  FDC->flags |= FDC->command[CMD_UNIT] & 1 ? SEEKDRVB_flag : SEEKDRVA_flag; // signal completion of seek operation
  FDC->phase = CMD_PHASE; // switch back to command phase (fdc_seek has no result phase!)
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_readtrk(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_readtrk
 *
 ***********************************************************************/
{
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_track->sectors != 0) // track is formatted?
    {
      FDC->command[CMD_R] = 1; // set sector ID to 1
      FDC->active_drive->current_sector = 0; // reset sector table index

      fdc_cmd_readtrk(FDC);
    }
    else // unformatted track
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x01; // Missing AM

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
  }
  else // drive was not ready
  {
    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_write(tFDC* FDC,
                       tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_write
 *
 ***********************************************************************/
{
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_drive->write_protected) // is write protect tab set?
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x02; // Not Writable

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
    else if (FDC->active_track->sectors != 0) // track is formatted?
    {
      fdc_cmd_write(FDC);
    }
    else // unformatted track
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x01; // Missing AM

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
  }
  else // drive was not ready
  {
    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_read(tFDC* FDC,
                      tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_read
 *
 ***********************************************************************/
{
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_track->sectors != 0) // track is formatted?
    {
      fdc_cmd_read(FDC);
    }
    else // unformatted track
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x01; // Missing AM

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
  }
  else // drive was not ready
  {
    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_readID(tFDC* FDC,
                        tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_readID
 *
 ***********************************************************************/
{
tULong idx;
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_track->sectors != 0) // track is formatted?
    {
      idx = FDC->active_drive->current_sector; // get the active sector index

      if (idx >= FDC->active_track->sectors) // index beyond number of sectors for this track?
      {
        idx = 0; // reset index
      }

      // FDC->result = tULong*, FDC->active_track->sector[idx].CHRN = tUChar*
      FDC->result[RES_C] = FDC->active_track->sector[idx].CHRN[0];
      FDC->result[RES_C+1] = FDC->active_track->sector[idx].CHRN[1];
      FDC->result[RES_C+2] = FDC->active_track->sector[idx].CHRN[2];
      FDC->result[RES_C+3] = FDC->active_track->sector[idx].CHRN[3];

      FDC->active_drive->current_sector = idx + 1; // update sector table index for active drive
    }
    else // unformatted track
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x01; // Missing AM

      LOAD_RESULT_WITH_CHRN;
    }
  }

  FDC->phase = RESULT_PHASE; // switch to result phase
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_writeID(tFDC* FDC,
                         tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_writeID
 *
 ***********************************************************************/
{
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_drive->write_protected) // is write protect tab set?
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x02; // Not Writable

      LOAD_RESULT_WITH_CHRN

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
    else
    {
      FDC->buffer_count = FDC->command[CMD_H] << 2; // number of sectors * 4 = number of bytes still outstanding
      FDC->buffer_ptr = NativeCPC->pbGPBuffer; // buffer to temporarily hold the track format
      FDC->buffer_endptr = NativeCPC->pbGPBuffer + FDC->buffer_count;
      FDC->timeout = INITIAL_TIMEOUT;
      FDC->read_status_delay = 1;
    }
  }
  else // drive was not ready
  {
    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_scan(tFDC* FDC,
                      tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  fdc_scan
 *
 ***********************************************************************/
{
tULong side;

  FDC->led = 1; // turn the drive LED on

  FDC_CHECK_UNIT;

  if (fdc_init_status_regs(FDC) == 0) // drive Ready?
  {
    FDC->active_drive->current_side = (FDC->command[CMD_UNIT] & 4) >> 2; // extract target side
    side = FDC->active_drive->sides ? FDC->active_drive->current_side : 0; // single sided drives only acccess side 1

    FDC->active_track = &FDC->active_drive->track[FDC->active_drive->current_track][side];

    if (FDC->active_track->sectors != 0) // track is formatted?
    {
      if (FDC->command[CMD_STP] > 2)
      {
        FDC->command[CMD_STP] = 2; // step can only be 1 or 2
      }

      FDC->flags |= SCAN_flag; // scan command active
      fdc_cmd_scan(FDC);
    }
    else // unformatted track
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x01; // Missing AM

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }
  }
  else // drive was not ready
  {
    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_cmd_readtrk(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_cmd_readtrk
 *
 ***********************************************************************/
{
tSector* sector;
tLong sector_size;

  sector = &FDC->active_track->sector[FDC->active_drive->current_sector];

  if ( (sector->CHRN[0] == FDC->command[CMD_C]) &&
       (sector->CHRN[1] == FDC->command[CMD_C+1]) &&
       (sector->CHRN[2] == FDC->command[CMD_C+2]) &&
       (sector->CHRN[3] == FDC->command[CMD_C+3]) )
  {
    FDC->result[RES_ST1] |= 0x04; // No Data
  }

  FDC->result[RES_ST2] &= 0xbf; // clear Control Mark, if it was set
  FDC->result[RES_ST1] |= sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
  FDC->result[RES_ST2] |= sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits

  if (FDC->command[CMD_N] == 0) // use DTL for length?
  {
    sector_size = FDC->command[CMD_DTL]; // size of sector is defined by DTL value

    if (sector_size > 0x80)
    {
      sector_size = 0x80; // max DTL value is 128
    }
  }
  else
  {
    sector_size = 128 << FDC->command[CMD_N]; // determine number of bytes from N value
  }

  FDC->buffer_count = sector_size; // init number of bytes to transfer
  FDC->buffer_ptr = sector->data; // pointer to sector data
  FDC->buffer_endptr = FDC->active_track->data + FDC->active_track->size; // pointer beyond end of track data
  FDC->timeout = INITIAL_TIMEOUT;
  FDC->read_status_delay = 1;
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_cmd_read(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_cmd_read
 *
 ***********************************************************************/
{
tSector* sector;
tULong sector_size;
tULong loop = 1;

  do
  {
    sector = fdc_find_sector(FDC,
                             &FDC->command[CMD_C]); // locate the requested sector on the current track

    if (sector) // sector found
    {
      FDC->result[RES_ST1] = sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
      FDC->result[RES_ST2] = sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits

      if (FDC->command[CMD_CODE] == 0x4c) // read deleted data command?
      {
        FDC->result[RES_ST2] ^= 0x40; // invert Control Mark
      }

      if ((FDC->flags & SKIP_flag) && (FDC->result[RES_ST2] &= 0x40)) // skip sector?
      {
        if (FDC->command[CMD_R] != FDC->command[CMD_EOT]) // continue looking?
        {
          FDC->command[CMD_R]++; // advance to next sector
          continue;
        }
        else // no data to transfer -> no execution phase
        {
          LOAD_RESULT_WITH_STATUS;
          LOAD_RESULT_WITH_CHRN;

          FDC->phase = RESULT_PHASE; // switch to result phase
        }
      }
      else // sector data is to be transferred
      {
        if (FDC->result[RES_ST2] &= 0x40) // does the sector have an AM opposite of what we want?
        {
          FDC->command[CMD_EOT] = FDC->command[CMD_R]; // execution ends on this sector
        }

        if (FDC->command[CMD_N] == 0) // use DTL for length?
        {
          sector_size = FDC->command[CMD_DTL]; // size of sector is defined by DTL value

          if (sector_size > 0x80)
          {
            sector_size = 0x80; // max DTL value is 128
          }
        }
        else
        {
          sector_size = 128 << FDC->command[CMD_N]; // determine number of bytes from N value
        }

        FDC->buffer_count = sector_size; // init number of bytes to transfer
        FDC->buffer_ptr = sector->data; // pointer to sector data
        FDC->buffer_endptr = FDC->active_track->data + FDC->active_track->size; // pointer beyond end of track data
        FDC->timeout = INITIAL_TIMEOUT;
        FDC->read_status_delay = 1;
      }
    }
    else // sector not found
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x04; // No Data

      LOAD_RESULT_WITH_CHRN;

      FDC->phase = RESULT_PHASE; // switch to result phase
    }

    loop = 0;
  }
  while (loop);
}
/*----------------------------------------------------------------------------*/


static tSector* fdc_find_sector(tFDC* FDC,
                                tULong* requested_CHRN)
/***********************************************************************
 *
 *  fdc_find_sector
 *
 ***********************************************************************/
{
tSector *sector;
tULong loop_count;
tULong idx;
tULong cylinder;

  sector = cNull; // return value indicates 'sector not found' by default
  loop_count = 0; // detection of index hole counter
  idx = FDC->active_drive->current_sector; // get the active sector index

  do
  {
    if ( (requested_CHRN[0] == FDC->active_track->sector[idx].CHRN[0]) &&
         (requested_CHRN[1] == FDC->active_track->sector[idx].CHRN[1]) &&
         (requested_CHRN[2] == FDC->active_track->sector[idx].CHRN[2]) &&
         (requested_CHRN[3] == FDC->active_track->sector[idx].CHRN[3]) )
    {
      sector = &FDC->active_track->sector[idx]; // return value points to sector information

      if ((sector->flags[0] & 0x20) || (sector->flags[1] & 0x20)) // any Data Errors?
      {
        if (FDC->active_drive->random_DEs) // simulate 'random' DEs?
        {
          FDC->flags |= RNDDE_flag;
        }
      }
      FDC->result[RES_ST2] &= ~(0x02 | 0x10); // remove possible Bad Cylinder + No Cylinder flags
      break;
    }

    cylinder = FDC->active_track->sector[idx].CHRN[0]; // extract C
    if (cylinder == 0xff)
    {
      FDC->result[RES_ST2] |= 0x02; // Bad Cylinder
    }
    else if (cylinder != FDC->command[CMD_C]) // does not match requested C?
    {
      FDC->result[RES_ST2] |= 0x10; // No Cylinder
    }
    idx++; // increase sector table index
    if (idx >= FDC->active_track->sectors) // index beyond number of sectors for this track?
    {
      idx = 0; // reset index
      loop_count++; // increase 'index hole' count
    }
  }
  while (loop_count < 2); // loop until sector is found, or index hole has passed twice

  if (FDC->result[RES_ST2] & 0x02) // Bad Cylinder set?
  {
    FDC->result[RES_ST2] &= ~0x10; // remove possible No Cylinder flag
  }

  FDC->active_drive->current_sector = idx; // update sector table index for active drive

  return sector;
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_cmd_scan(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_cmd_scan
 *
 ***********************************************************************/
{
tSector *sector;
tULong sector_size;
tULong loop = 1;

  do
  {
    sector = fdc_find_sector(FDC,
                             &FDC->command[CMD_C]); // locate the requested sector on the current track

    // sector found
    if (sector)
    {
      FDC->result[RES_ST1] = sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
      FDC->result[RES_ST2] = sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits

      if ((FDC->flags & SKIP_flag) && (FDC->result[RES_ST2] &= 0x40)) // skip sector?
      {
        if (FDC->command[CMD_R] != FDC->command[CMD_EOT]) // continue looking?
        {
          FDC->command[CMD_R] += FDC->command[CMD_STP]; // advance to next sector
          continue;
        }
        else // no data to transfer -> no execution phase
        {
          LOAD_RESULT_WITH_STATUS;
          LOAD_RESULT_WITH_CHRN;

          FDC->phase = RESULT_PHASE; // switch to result phase
        }
      }
      else // sector data is to be transferred
      {
        if (FDC->result[RES_ST2] &= 0x40) // does the sector have an AM opposite of what we want?
        {
          FDC->command[CMD_EOT] = FDC->command[CMD_R]; // execution ends on this sector
        }

        sector_size = 128 << FDC->command[CMD_N]; // determine number of bytes from N value

        FDC->buffer_count = sector_size; // init number of bytes to transfer
        FDC->buffer_ptr = sector->data; // pointer to sector data
        FDC->buffer_endptr = FDC->active_track->data + FDC->active_track->size; // pointer beyond end of track data
        FDC->flags &= ~SCANFAILED_flag; // reset scan failed flag
        FDC->result[RES_ST2] |= 0x08; // assume data matches: set Scan Equal Hit
        FDC->timeout = INITIAL_TIMEOUT;
        FDC->read_status_delay = 1;
      }
    }

    // sector not found
    else
    {
      FDC->result[RES_ST0] |= 0x40; // AT
      FDC->result[RES_ST1] |= 0x04; // No Data

      LOAD_RESULT_WITH_CHRN

      FDC->phase = RESULT_PHASE; // switch to result phase
    }

    loop = 0;
  }
  while (loop);
}
/*----------------------------------------------------------------------------*/


static tVoid fdc_cmd_write(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_cmd_write
 *
 ***********************************************************************/
{
tSector *sector;
tLong sector_size;

  sector = fdc_find_sector(FDC,
                           &FDC->command[CMD_C]); // locate the requested sector on the current track

  //
  // sector found
  //
  if (sector)
  {
    sector->flags[0] = 0; // clear ST1 for this sector

    if (FDC->command[CMD_CODE] == 0x45) // write data command?
    {
      sector->flags[1] = 0; // clear ST2
    }
    else // write deleted data
    {
      sector->flags[1] = 0x40; // set Control Mark
    }

    if (FDC->command[CMD_N] == 0) // use DTL for length?
    {
      sector_size = FDC->command[CMD_DTL]; // size of sector is defined by DTL value
      if (sector_size > 0x80)
      {
        sector_size = 0x80; // max DTL value is 128
      }
    }
    else
    {
      sector_size = 128 << FDC->command[CMD_N]; // determine number of bytes from N value
    }

    FDC->buffer_count = sector_size; // init number of bytes to transfer
    FDC->buffer_ptr = sector->data; // pointer to sector data
    FDC->buffer_endptr = FDC->active_track->data + FDC->active_track->size; // pointer beyond end of track data
    FDC->timeout = INITIAL_TIMEOUT;
    FDC->read_status_delay = 1;
  }

  // sector not found
  else
  {
    FDC->result[RES_ST0] |= 0x40; // AT
    FDC->result[RES_ST1] |= 0x04; // No Data

    LOAD_RESULT_WITH_CHRN;

    FDC->phase = RESULT_PHASE; // switch to result phase
  }
}
/*----------------------------------------------------------------------------*/


static tULong fdc_init_status_regs(tFDC* FDC)
/***********************************************************************
 *
 *  fdc_init_status_regs
 *
 ***********************************************************************/
{
tULong val;

  MemSetLong(FDC->result,
             NUMBER_OF_ITEMS(FDC->result),
             0); // clear result codes buffer

  val = FDC->command[CMD_UNIT] & 7; // keep head and unit of command

  if ((FDC->active_drive->tracks == 0) || (!FDC->motor)) // no DSK in the drive, or drive motor is turned off?
  {
     val |= 0x48; // Abnormal Termination + Not Ready
  }

  FDC->result[RES_ST0] = val; // write ST0 to result codes buffer

  return (val & 8); // return value indicates whether drive is ready (0) or not (8)
}
/*----------------------------------------------------------------------------*/


//
// TAPE
//

#ifdef ENABLE_TAPE

inline tVoid Tape_GetCycleCount(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_GetCycleCount
 *
 ***********************************************************************/
{
  NativeCPC->dwTapePulseCycles = CYCLE_ADJUST(*NativeCPC->pwTapePulseTablePtr++);

  if (NativeCPC->pwTapePulseTablePtr >= NativeCPC->pwTapePulseTableEnd)
  {
    NativeCPC->pwTapePulseTablePtr = NativeCPC->pwTapePulseTable;
  }
}
/*----------------------------------------------------------------------------*/


inline tVoid Tape_SwitchLevel(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_SwitchLevel
 *
 ***********************************************************************/
{
  NativeCPC->PSG->bTapeLevel = (NativeCPC->PSG->bTapeLevel == TAPE_LEVEL_LOW) ? TAPE_LEVEL_HIGH : TAPE_LEVEL_LOW;
}
/*----------------------------------------------------------------------------*/


static tLong Tape_ReadDataBit(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_ReadDataBit
 *
 ***********************************************************************/
{
  if (NativeCPC->dwTapeDataCount)
  {
    if (!NativeCPC->dwTapeBitsToShift)
    {
      NativeCPC->bTapeData = *NativeCPC->pbTapeBlockData; // get the next data byte
      NativeCPC->pbTapeBlockData++;
      NativeCPC->dwTapeBitsToShift = 8;
    }

    NativeCPC->bTapeData <<= 1;
    NativeCPC->dwTapeBitsToShift--;
    NativeCPC->dwTapeDataCount--;

    if (NativeCPC->bTapeData & 0x80)
    {
      NativeCPC->dwTapePulseCycles = NativeCPC->dwTapeOnePulseCycles;
    }
    else
    {
      NativeCPC->dwTapePulseCycles = NativeCPC->dwTapeZeroPulseCycles;
    }

    NativeCPC->dwTapePulseCount = 2; // two pulses = one bit

    return 1;
  }

  return 0; // no more data
}
/*----------------------------------------------------------------------------*/


static tLong Tape_ReadSampleDataBit(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_ReadSampleDataBit
 *
 ***********************************************************************/
{
  if (NativeCPC->dwTapeDataCount)
  {
    if (!NativeCPC->dwTapeBitsToShift)
    {
      NativeCPC->bTapeData = *NativeCPC->pbTapeBlockData; // get the next data byte
      NativeCPC->pbTapeBlockData++;
      NativeCPC->dwTapeBitsToShift = 8;
    }

    NativeCPC->bTapeData <<= 1;
    NativeCPC->dwTapeBitsToShift--;
    NativeCPC->dwTapeDataCount--;

    if (NativeCPC->bTapeData & 0x80)
    {
      NativeCPC->PSG->bTapeLevel = TAPE_LEVEL_HIGH; // set high level
    }
    else
    {
      NativeCPC->PSG->bTapeLevel = TAPE_LEVEL_LOW; // set low level
    }

    NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level

    return 1;
  }

  return 0; // no more data
}
/*----------------------------------------------------------------------------*/


static tLong Tape_GetNextBlock(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_GetNextBlock
 *
 ***********************************************************************/
{
  while (NativeCPC->pbTapeBlock < NativeCPC->pbTapeImageEnd) // loop until a valid block is found
  {
    switch (*NativeCPC->pbTapeBlock)
    {
      case 0x10: // standard speed data block
        NativeCPC->dwTapeStage = TAPE_PILOT_STAGE; // block starts with a pilot tone
        NativeCPC->dwTapePulseCycles = CYCLE_ADJUST(2168);
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        NativeCPC->dwTapePulseCount = 3220;
        return 1;

      case 0x11: // turbo loading data block
        NativeCPC->dwTapeStage = TAPE_PILOT_STAGE; // block starts with a pilot tone
        NativeCPC->dwTapePulseCycles = CYCLE_ADJUST(*(tUShort *)(NativeCPC->pbTapeBlock+0x01));
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        NativeCPC->dwTapePulseCount = *(tUShort *)(NativeCPC->pbTapeBlock+0x01+0x0a);
        return 1;

      case 0x12: // pure tone
        NativeCPC->dwTapeStage = TAPE_PILOT_STAGE; // block starts with a pilot tone
        NativeCPC->dwTapePulseCycles = CYCLE_ADJUST(*(tUShort *)(NativeCPC->pbTapeBlock+0x01));
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        NativeCPC->dwTapePulseCount = *(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02);
        return 1;

      case 0x13: // sequence of pulses of different length
        NativeCPC->dwTapeStage = TAPE_SYNC_STAGE;
        NativeCPC->dwTapePulseCount = *(NativeCPC->pbTapeBlock+0x01);
        NativeCPC->pwTapePulseTable =
        NativeCPC->pwTapePulseTablePtr = (tUShort*)(NativeCPC->pbTapeBlock+0x01+0x01);
        NativeCPC->pwTapePulseTableEnd = NativeCPC->pwTapePulseTable + NativeCPC->dwTapePulseCount;
        Tape_GetCycleCount(NativeCPC);
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        return 1;

      case 0x14: // pure data block
        NativeCPC->dwTapeStage = TAPE_DATA_STAGE;
        NativeCPC->dwTapeZeroPulseCycles = CYCLE_ADJUST(*(tUShort *)(NativeCPC->pbTapeBlock+0x01)); // pulse length for a zero bit
        NativeCPC->dwTapeOnePulseCycles = CYCLE_ADJUST(*(tUShort *)(NativeCPC->pbTapeBlock+0x01+0x02)); // pulse length for a one bit
        NativeCPC->dwTapeDataCount = ((*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x07) & 0x00ffffff) - 1) << 3; // (byte count - 1) * 8 bits
        NativeCPC->dwTapeDataCount += *(NativeCPC->pbTapeBlock+0x01+0x04); // add the number of bits in the last data byte
        NativeCPC->pbTapeBlockData = NativeCPC->pbTapeBlock+0x01+0x0a; // pointer to the tape data
        NativeCPC->dwTapeBitsToShift = 0;
        Tape_ReadDataBit(NativeCPC); // get the first bit of the first data byte
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        return 1;

      case 0x15: // direct recording
        NativeCPC->dwTapeStage = TAPE_SAMPLE_DATA_STAGE;
        NativeCPC->dwTapePulseCycles = CYCLE_ADJUST(*(tUShort *)(NativeCPC->pbTapeBlock+0x01)); // number of T states per sample
        NativeCPC->dwTapeDataCount = ((*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x05) & 0x00ffffff) - 1) << 3; // (byte count - 1) * 8 bits
        NativeCPC->dwTapeDataCount += *(NativeCPC->pbTapeBlock+0x01+0x04); // add the number of bits in the last data byte
        NativeCPC->pbTapeBlockData = NativeCPC->pbTapeBlock+0x01+0x08; // pointer to the tape data
        NativeCPC->dwTapeBitsToShift = 0;
        Tape_ReadSampleDataBit(NativeCPC); // get the first bit of the first data byte
        return 1;

      case 0x20: // pause
        if (*(tUShort *)(NativeCPC->pbTapeBlock+0x01)) // was a pause requested?
        {
          NativeCPC->dwTapeStage = TAPE_PAUSE_STAGE;
          NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(1); // start with a 1ms level opposite to the one last played
          NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
          NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(*(tUShort *)(NativeCPC->pbTapeBlock+0x01) - 1); // get the actual pause length
          NativeCPC->dwTapePulseCount = 2; // just one pulse
          return 1;
        }
        else
        {
          NativeCPC->pbTapeBlock += 2 + 1; // skip block if pause length is 0
        }
        break;

      case 0x21: // group start
        NativeCPC->pbTapeBlock += *(NativeCPC->pbTapeBlock+0x01) + 1 + 1; // nothing to do, skip the block
        break;

      case 0x22: // group end
        NativeCPC->pbTapeBlock += 1; // nothing to do, skip the block
        break;

      case 0x30: // text description
        NativeCPC->pbTapeBlock += *(NativeCPC->pbTapeBlock+0x01) + 1 + 1; // nothing to do, skip the block
        break;

      case 0x31: // message block
        NativeCPC->pbTapeBlock += *(NativeCPC->pbTapeBlock+0x01+0x01) + 2 + 1; // nothing to do, skip the block
        break;

      case 0x32: // archive info
        NativeCPC->pbTapeBlock += *(tUShort*)(NativeCPC->pbTapeBlock+0x01) + 2 + 1; // nothing to do, skip the block
        break;

      case 0x33: // hardware type
        NativeCPC->pbTapeBlock += (*(NativeCPC->pbTapeBlock+0x01) * 3) + 1 + 1; // nothing to do, skip the block
        break;

      case 0x34: // emulation info
        NativeCPC->pbTapeBlock += 8 + 1; // nothing to do, skip the block
        break;

      case 0x35: // custom info block
        NativeCPC->pbTapeBlock += *(tULong*)(NativeCPC->pbTapeBlock+0x01+0x10) + 0x14 + 1; // nothing to do, skip the block
        break;

      case 0x40: // snapshot block
        NativeCPC->pbTapeBlock += (*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x01) & 0x00ffffff) + 0x04 + 1; // nothing to do, skip the block
        break;

      case 0x5A: // another tzx/cdt file
        NativeCPC->pbTapeBlock += 9 + 1; // nothing to do, skip the block
        break;

      default: // "extension rule"
        NativeCPC->pbTapeBlock += *(tULong*)(NativeCPC->pbTapeBlock+0x01) + 4 + 1; // nothing to do, skip the block
    }
  }

  return 0; // we've reached the end of the image
}
/*----------------------------------------------------------------------------*/


static tVoid Tape_BlockDone(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_BlockDone
 *
 ***********************************************************************/
{
  if (NativeCPC->pbTapeBlock < NativeCPC->pbTapeImageEnd)
  {
    switch (*NativeCPC->pbTapeBlock)
    {
      case 0x10: // standard speed data block
        NativeCPC->pbTapeBlock += *(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02) + 0x04 + 1;
        break;

      case 0x11: // turbo loading data block
        NativeCPC->pbTapeBlock += (*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x0f) & 0x00ffffff) + 0x12 + 1;
        break;

      case 0x12: // pure tone
        NativeCPC->pbTapeBlock += 4 + 1;
        break;

      case 0x13: // sequence of pulses of different length
        NativeCPC->pbTapeBlock += *(NativeCPC->pbTapeBlock+0x01) * 2 + 1 + 1;
        break;

      case 0x14: // pure data block
        NativeCPC->pbTapeBlock += (*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x07) & 0x00ffffff) + 0x0a + 1;
        break;

      case 0x15: // direct recording
        NativeCPC->pbTapeBlock += (*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x05) & 0x00ffffff) + 0x08 + 1;
        break;

      case 0x20: // pause
        NativeCPC->pbTapeBlock += 2 + 1;
        break;
    }

    if (!Tape_GetNextBlock(NativeCPC))
    {
      NativeCPC->dwTapeStage = TAPE_END;
      NativeCPC->tape_play_button = 0;
    }
  }
}
/*----------------------------------------------------------------------------*/


static tVoid Tape_UpdateLevel(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  Tape_UpdateLevel
 *
 ***********************************************************************/
{
  switch (NativeCPC->dwTapeStage)
  {
    case TAPE_PILOT_STAGE:
      Tape_SwitchLevel(NativeCPC);
      NativeCPC->dwTapePulseCount--;
      if (NativeCPC->dwTapePulseCount > 0) // is the pilot tone still playing?
      {
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
      }
      else // finished with the pilot tone
      {
        switch (*NativeCPC->pbTapeBlock)
        {
          case 0x10: // standard speed data block
            NativeCPC->dwTapeStage = TAPE_SYNC_STAGE;
            NativeCPC->wCycleTable[0] = 667;
            NativeCPC->wCycleTable[1] = 735;
            NativeCPC->pwTapePulseTable =
            NativeCPC->pwTapePulseTablePtr = &NativeCPC->wCycleTable[0];
            NativeCPC->pwTapePulseTableEnd = &NativeCPC->wCycleTable[2];
            Tape_GetCycleCount(NativeCPC);
            NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
            NativeCPC->dwTapePulseCount = 2;
            break;

          case 0x11: // turbo loading data block
            NativeCPC->dwTapeStage = TAPE_SYNC_STAGE;
            NativeCPC->pwTapePulseTable =
            NativeCPC->pwTapePulseTablePtr = (tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02);
            NativeCPC->pwTapePulseTableEnd = (tUShort*)(NativeCPC->pbTapeBlock+0x01+0x06);
            Tape_GetCycleCount(NativeCPC);
            NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
            NativeCPC->dwTapePulseCount = 2;
            break;

          case 0x12: // pure tone
            Tape_BlockDone(NativeCPC);
            break;
        }
      }
      break;

    case TAPE_SYNC_STAGE:
      Tape_SwitchLevel(NativeCPC);
      NativeCPC->dwTapePulseCount--;
      if (NativeCPC->dwTapePulseCount > 0)
      {
        Tape_GetCycleCount(NativeCPC);
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
      }
      else
      {
        switch (*NativeCPC->pbTapeBlock)
        {
          case 0x10: // standard speed data block
            NativeCPC->dwTapeStage = TAPE_DATA_STAGE;
            NativeCPC->dwTapeZeroPulseCycles = CYCLE_ADJUST(855); // pulse length for a zero bit
            NativeCPC->dwTapeOnePulseCycles = CYCLE_ADJUST(1710); // pulse length for a one bit
            NativeCPC->dwTapeDataCount = *(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02) << 3; // byte count * 8 bits;
            NativeCPC->pbTapeBlockData = NativeCPC->pbTapeBlock+0x01+0x04; // pointer to the tape data
            NativeCPC->dwTapeBitsToShift = 0;
            Tape_ReadDataBit(NativeCPC);
            NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
            break;

          case 0x11: // turbo loading data block
            NativeCPC->dwTapeStage = TAPE_DATA_STAGE;
            NativeCPC->dwTapeZeroPulseCycles = CYCLE_ADJUST(*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x06)); // pulse length for a zero bit
            NativeCPC->dwTapeOnePulseCycles = CYCLE_ADJUST(*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x08)); // pulse length for a one bit
            NativeCPC->dwTapeDataCount = ((*(tULong*)(NativeCPC->pbTapeBlock+0x01+0x0f) & 0x00ffffff) - 1) << 3; // (byte count - 1) * 8 bits;
            NativeCPC->dwTapeDataCount += *(NativeCPC->pbTapeBlock+0x01+0x0c); // add the number of bits in the last data byte
            NativeCPC->pbTapeBlockData = NativeCPC->pbTapeBlock+0x01+0x12; // pointer to the tape data
            NativeCPC->dwTapeBitsToShift = 0;
            Tape_ReadDataBit(NativeCPC);
            NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
            break;

          case 0x13: // sequence of pulses of different length
            Tape_BlockDone(NativeCPC);
            break;
        }
      }
      break;

    case TAPE_DATA_STAGE:
      Tape_SwitchLevel(NativeCPC);
      NativeCPC->dwTapePulseCount--;
      if (NativeCPC->dwTapePulseCount > 0)
      {
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
      }
      else
      {
        if (Tape_ReadDataBit(NativeCPC))
        {
          NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
        }
        else
        {
          switch (*NativeCPC->pbTapeBlock)
          {
            case 0x10: // standard speed data block
              if (*(tUShort*)(NativeCPC->pbTapeBlock+0x01)) // was a pause requested?
              {
                NativeCPC->dwTapeStage = TAPE_PAUSE_STAGE;
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(1); // start with a 1ms level opposite to the one last played
                NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(*(tUShort*)(NativeCPC->pbTapeBlock+0x01) - 1); // pause in ms
                NativeCPC->dwTapePulseCount = 2; // just one pulse
              }
              else
              {
                Tape_BlockDone(NativeCPC);
              }
              break;

            case 0x11: // turbo loading data block
              if (*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x0d)) // was a pause requested?
              {
                NativeCPC->dwTapeStage = TAPE_PAUSE_STAGE;
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(1); // start with a 1ms level opposite to the one last played
                NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x0d) - 1); // pause in ms
                NativeCPC->dwTapePulseCount = 2; // just one pulse
              }
              else
              {
                Tape_BlockDone(NativeCPC);
              }
              break;

            case 0x14: // pure data block
              if (*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x05)) // was a pause requested?
              {
                NativeCPC->dwTapeStage = TAPE_PAUSE_STAGE;
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(1); // start with a 1ms level opposite to the one last played
                NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
                NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x05) - 1); // pause in ms
                NativeCPC->dwTapePulseCount = 2; // just one pulse
              }
              else
              {
                Tape_BlockDone(NativeCPC);
              }
              break;

            default:
              Tape_BlockDone(NativeCPC);
          }
        }
      }
      break;

    case TAPE_SAMPLE_DATA_STAGE:
      if (!Tape_ReadSampleDataBit(NativeCPC))
      {
        if (*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02)) // was a pause requested?
        {
          NativeCPC->dwTapeStage = TAPE_PAUSE_STAGE;
          NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(1); // start with a 1ms level opposite to the one last played
          NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
          NativeCPC->dwTapePulseCycles = MS_TO_CYCLES(*(tUShort*)(NativeCPC->pbTapeBlock+0x01+0x02) - 1); // pause in ms
          NativeCPC->dwTapePulseCount = 2; // just one pulse
        }
        else
        {
          Tape_BlockDone(NativeCPC);
        }
      }
      break;

    case TAPE_PAUSE_STAGE:
      NativeCPC->PSG->bTapeLevel = TAPE_LEVEL_LOW;
      NativeCPC->dwTapePulseCount--;
      if (NativeCPC->dwTapePulseCount > 0)
      {
        NativeCPC->iTapeCycleCount += (tLong)NativeCPC->dwTapePulseCycles; // set cycle count for current level
      }
      else
      {
        Tape_BlockDone(NativeCPC);
      }
      break;

    case TAPE_END:
      NativeCPC->tape_play_button = 0;
      break;
  }
}
/*----------------------------------------------------------------------------*/

#endif /* ENABLE_TAPE */



//==============================================================================
//
// Routines
//
//==============================================================================
static inline tVoid MemMoveByte(tUChar* destP,
                                tUChar* sourceP,
                                tULong numBytes)
/***********************************************************************
 *
 *  MemMoveByte
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = *(sourceP++);
  }
}
/*----------------------------------------------------------------------------*/


static inline tVoid MemSetByte(tUChar* destP,
                               tULong numBytes,
                               tULong value)
/***********************************************************************
 *
 *  MemSetByte
 *
 ***********************************************************************/
{
  while (numBytes--)
  {
    *(destP++) = value;
  }
}
/*----------------------------------------------------------------------------*/


static inline tVoid MemSetLong(tULong* destP,
                               tULong numLongs,
                               tULong value)
/***********************************************************************
 *
 *  MemSetLong
 *
 ***********************************************************************/
{
  while (numLongs--)
  {
    *(destP++) = value;
  }
}
/*----------------------------------------------------------------------------*/


//==============================================================================
//
// Unitary Tests
//
//==============================================================================
#ifdef _TESTU
// Prototypes of TestU fonctions
static tUShort TestU_MemSetByte_1(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_MemSetLong_1(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_MemMoveByte_1(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_Compiler_1(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_Compiler_2(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_Compiler_3(tNativeCPC* NativeCPC, tUChar NoTest);
static tUShort TestU_Compiler_4(tNativeCPC* NativeCPC, tUChar NoTest);


static tUShort PerformTestU(tNativeCPC* NativeCPC)
/***********************************************************************
 *
 *  PerformTestU
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar NoTest = 1;

  /*  1 */ if (Result == errNone) Result = TestU_MemSetByte_1(NativeCPC, NoTest++);
  /*  2 */ if (Result == errNone) Result = TestU_MemSetLong_1(NativeCPC, NoTest++);
  /*  3 */ if (Result == errNone) Result = TestU_MemMoveByte_1(NativeCPC, NoTest++);
  /*  4 */ if (Result == errNone) Result = TestU_Compiler_1(NativeCPC, NoTest++);
  /*  5 */ if (Result == errNone) Result = TestU_Compiler_2(NativeCPC, NoTest++);
  /*  6 */ if (Result == errNone) Result = TestU_Compiler_3(NativeCPC, NoTest++);
  /*  7 */ if (Result == errNone) Result = TestU_Compiler_4(NativeCPC, NoTest++);

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_MemSetByte_1(tNativeCPC* NativeCPC,
                                  tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemSetByte_1
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar ArrayA[100];
tUChar Loop;

  NOT_USED(NativeCPC);

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
    ArrayA[Loop] = 0x55;

  // Perform operation
  MemSetByte(ArrayA,
             95,
             0xAA);

  // Check Result
  for (Loop=0; Loop<95; Loop++)
  {
    if (ArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (ArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_MemSetLong_1(tNativeCPC* NativeCPC,
                                  tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemSetLong_1
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tULong ArrayA[100];
tUChar Loop;

  NOT_USED(NativeCPC);

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
    ArrayA[Loop] = 0x55;

  // Perform operation
  MemSetLong(ArrayA,
             95,
             0xAA);

  // Check Result
  for (Loop=0; Loop<95; Loop++)
  {
    if (ArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (ArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_MemMoveByte_1(tNativeCPC* NativeCPC,
                                   tUChar NoTest)
/***********************************************************************
 *
 *  TestU_MemMoveByte_1
 *
 ***********************************************************************/
{
tUShort Result = errNone;
tUChar SrcArrayA[100];
tUChar DstArrayA[100];
tUChar Loop;

  NOT_USED(NativeCPC);

  // Prepare conditions
  for (Loop=0; Loop<100; Loop++)
  {
    SrcArrayA[Loop] = 0x55;
    DstArrayA[Loop] = 0xAA;
  }

  // Perform operation
  MemMoveByte(DstArrayA,
              SrcArrayA,
              95);

  // Check Result
  for (Loop=0; Loop<100; Loop++)
  {
    if (SrcArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=0; Loop<95; Loop++)
  {
    if (DstArrayA[Loop] != 0x55)
      Result=testUErrorClass+NoTest;
  }
  for (Loop=95; Loop<100; Loop++)
  {
    if (DstArrayA[Loop] != 0xAA)
      Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_Compiler_1(tNativeCPC* NativeCPC,
                                tUChar NoTest)
/***********************************************************************
 *
 *  TestU_Compiler_1
 *
 ***********************************************************************/
{
tLong longValue;
tUShort Result = errNone;
tChar charValue;

  NOT_USED(NativeCPC);

  // Test 1 : signed char => signed long
  charValue = -116;
  longValue = (tLong)charValue;
  if (longValue != -116)
  {
    Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_Compiler_2(tNativeCPC* NativeCPC,
                                tUChar NoTest)
/***********************************************************************
 *
 *  TestU_Compiler_2
 *
 ***********************************************************************/
{
tULong ulongValue;
tUShort Result = errNone;
tUChar ucharValue;

  NOT_USED(NativeCPC);

  // Test 2 : unsigned char => unsigned long
  ucharValue = 140;
  ulongValue = (tULong)ucharValue;
  if (ulongValue != 140)
  {
    Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_Compiler_3(tNativeCPC* NativeCPC,
                                tUChar NoTest)
/***********************************************************************
 *
 *  TestU_Compiler_3
 *
 ***********************************************************************/
{
tLong longValue;
tUShort Result = errNone;
tUChar ucharValue;

  NOT_USED(NativeCPC);

  // Test 3 : unsigned char => signed long
  //
  // ATTENTION : unsigned char <> signed long
  //
  ucharValue = (tUChar)-116;
  longValue = (tLong)ucharValue;
  if (longValue != 140)
  {
    Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/


static tUShort TestU_Compiler_4(tNativeCPC* NativeCPC,
                                tUChar NoTest)
/***********************************************************************
 *
 *  TestU_Compiler_4
 *
 ***********************************************************************/
{
tULong ulongValue;
tUShort Result = errNone;
tChar charValue;

  NOT_USED(NativeCPC);

  // Test 4 : signed char => unsigned long
  //
  // ATTENTION : signed char <> unsigned long
  //
  charValue = (tChar)0x8c;
  ulongValue = (tULong)charValue;
  if (ulongValue != 0xffffff8c)
  {
    Result=testUErrorClass+NoTest;
  }

  return (Result);
}
/*----------------------------------------------------------------------------*/

#endif /* _TESTU */


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif
