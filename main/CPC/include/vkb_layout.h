/* Virtual keyboard layout */

#ifndef __VKB_LAYOUT_H
#define __VKB_LAYOUT_H

#include "types.h"
#include "sections.h"

typedef enum
{
  KeyStatic,
  KeyImpulse,
  KeyToggle,
} tKeyMode;

typedef struct
{
  tUShort Left;
  tUShort Top;
  tUShort Right;
  tUShort Bottom;
  tUChar nb_keys;
} tKeySection;

typedef struct
{
  tUShort Left;
  tUShort Top;
  tUShort Width;
  tUShort Height;
  DmResID resKeyUp;
  DmResID resKeyDown;
  tKeyMode KeyMode;
  tUChar CPCKey;
  tUChar charCode_Normal;
  tUChar charCode_Shift;
  void (*OnPushedP)(void);
  void (*OnReleasedP)(void);
  tUChar NextLeft;
  tUChar NextRight;
  tUChar NextUp;
  tUChar NextDown;
  tUChar yBase;
} tKeySetting;

// Number of keys
#define CPC_KB_KEYS 74
#define CPC_KB_SECTIONS 8

// Keyboard layout
extern const tKeySection cpc_ks[];
extern const tKeySetting cpc_kb[];

// Default key
extern const tKeySetting *CPC_DEFAULT_KEY;

#endif /* __VKB_LAYOUT_H */
