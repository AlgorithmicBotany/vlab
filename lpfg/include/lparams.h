#ifndef __LPARAMS_H__
#define __LPARAMS_H__

/*
        L-params:
        these are the declaration required by all three components:
        L2C, Generate and lsys.i (l2c-ed L file) to compile
*/

enum {
  __lc_eMaxIdentifierLength = 32,
  __lc_eMaxParams = 64,
  __lc_eMaxFormalModules = 32,
  __lc_eFirstModuleId = 2
};

enum __lc_GroupType { __lc_gtUnspecified, __lc_gtLsystem, __lc_gtGillespie };

#define StartBranchIdent "SB"
#define EndBranchIdent "EB"

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
