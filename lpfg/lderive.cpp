/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "lengine.h"
#include "succstor.h"
#include "comlineparam.h"
#include "gillespie.h"
#include "ContextScanner.h"
#include "PerformanceMonitor.h"
#include <iostream>
// Including float.h under Windows for _finite
#ifdef WIN32
#include <float.h>
#endif

void LEngine::DeriveString() {
  // derive the string from the axiom
  // by performing the specified number of steps

  if (ValidLsystem()) {
    TIME_THIS(0);
    _step = 0;
    _GillespieTime = 0.0;
    // empty both strings
    _lstring.Clear();
    _derivedstring.Clear();
    // execute the Start statement
    _dll.Start();

    // determined the requested number of derivation steps
    int s = _dll.DerivationLength();

    // initialize the string
    // with the axiom
    Axiom();

    try {
      while (_step < s && !StopRequested()) {
        // perform one derivation step
        Derive();
      }
    } catch (const char *msg) {
      // report any errors
      Utils::Error(msg);
    }

    // execute the End statement
    Stop();
  }

  if (comlineparam.TimedMode()) {
    std::ofstream target("PerformanceLog.txt");
    PerformanceMonitor::Report(target);
  }

  if (comlineparam.InterpretToFile()) {
    std::string targetFilename;
    BuildOutputFilename(targetFilename, -1);
    InterpretToFile(targetFilename);
  }
}

void LEngine::DecomposeString(const Lstring &src, Lstring &trg, int depth,
                              int group) const {
  ASSERT(ValidLsystem());
  LstringIterator iter(src);

  if (!iter.AtEnd())
    do {
      // will contain the pointers
      // to the parameters of the module
      // in the predecessor

      // assume no production applied yet
      bool applied = false;

      // if the current group is not default (0)
      // first try the current group
      if (group != 0) {
        applied = TryDecompose(iter, group);
      }
      // if decomposition didn't apply
      // or was not found try default group
      if (!applied) {
        applied = TryDecompose(iter, 0);
      }

      // if no matching interpretation rule
      // can be found
      if (!applied)
        // just append the current module to the new string
        trg.Append(iter);
      // otherwise
      else {
        // if maximum decomposition depth not reached
        if (depth > 0) {
          // create a new (temporary) string
          Lstring lstr(&_pool);
          // fill it with the contents
          // of the decomposition rule's
          // successor
          lstr.Add(Interface::GetSuccessorStorage());
          // and decompose the successor recursively
          DecomposeString(lstr, trg, depth - 1, group);
        }
        // otherwise (max depth exceeded)
        else {
          // just append the current module
          // to the new string
          trg.Append(iter);
          // clear the successor storage
          Interface::GetSuccessorStorage().Clear();
          // and print the warning message
          Utils::Message("Maximum decomposition depth reached for module %s\n",
                         iter.GetModuleName());
        }
      }
    }
    // advance to the next module in the string
    // and continue
    while (++iter);
}

bool LEngine::TryDecompose(const LstringIterator &citer, int iGroup) const {
  MatchDecompositionParameters matchParameters(iGroup, _dll);
  __lc_CallerData cd;
  return FindMatch(citer, cd, matchParameters);
}

bool LEngine::TryInterpret(const LstringIterator &citer, int iGroup,
                           int iVGroup) const {
  MatchInterpretationParameters matchParameters(iGroup, iVGroup, _dll);
  __lc_CallerData cd;
  return FindMatch(citer, cd, matchParameters);
}

void LEngine::Derive() {
  // perform one derivation step
  if (ValidLsystem()) {
    // if environment on
    if (!_dll.IgnoreEnvironment())
      // perform the environment interpretation step
      InterpretForEnvironment(_dll.CurrentGroup());

    // advance the step counter
    ++_step;
    // execute StartEach statement
    _dll.StartEach();
    // determine the current table id
    const int grp = _dll.CurrentGroup();
    // perform derivation
    // in the direction specified by the user

    switch (_dll.DerivationType(grp)) {
    case __lc_gtLsystem: {
      if (_dll.IsForward())
        DeriveForward(grp);
      else
        DeriveBackward(grp);
    } break;
    case __lc_gtGillespie: {
      if (_dll.IsForward())
        DeriveGForward(grp);
      else
        Utils::Message("Deriving Gillespie backward not supported\n.");
    } break;
    default:
      Utils::Message("enumeration value '__lc_gtUnspecified' not handled\n");
      break;
    }

    // clear the old string
    _derivedstring.Clear();

    // if there are any decompositions
    if (HasDecompositions()) {
      try {
        // decompose
        DecomposeString(_lstring, _derivedstring, _dll.DecompositionMaxDepth(),
                        grp);
        _lstring.Swap(_derivedstring);
        _derivedstring.Clear();
      } catch (Utils::SEWrapper sew) {
#ifdef _WINDOWS
        switch (sew.n) {
        case EXCEPTION_STACK_OVERFLOW:
          Utils::Error(
              "Process stack overflow, string decomposition aborted\n");
          Utils::Error("Restart LPFG\n");
          _lstring.Clear();
          _derivedstring.Clear();
          break;
        }
#endif
      }
    }
    // execute EndEach
    _dll.EndEach();

    if (StopRequested())
      _dll.End();

    if (comlineparam.ToStringEveryStep()) {
      std::string filename;
      BuildOutputFilename(filename, _step);
      InterpretToFile(filename);
    }
  }
}

void LEngine::Axiom() {

  ASSERT(ValidLsystem());
  Debug("Executing axiom\n");
  // reset stop flag
  // (can be set by the Stop function from L-system)
  _stopFlag = false;

  // execute axiom
  _dll.Axiom();
  // initialize the string with the axiom
  _lstring.Add(Interface::GetSuccessorStorage());
  _derivedstring.Clear();
  // if there are any decompositions
  if (HasDecompositions()) {
    try {
      // decompose the string
      DecomposeString(_lstring, _derivedstring, _dll.DecompositionMaxDepth(),
                      _dll.CurrentGroup());
      _lstring.Swap(_derivedstring);
      _derivedstring.Clear();
    } catch (Utils::SEWrapper sew) {
#ifdef _WINDOWS
      switch (sew.n) {
      case EXCEPTION_STACK_OVERFLOW:
        Utils::Error("Process stack overflow, string decomposition aborted\n");
        Utils::Error("Restart LPFG\n");
        _lstring.Clear();
        _derivedstring.Clear();
        break;
      }
#endif
    }
  }
  if (comlineparam.DebugMode()) {
    Utils::Message("Dumping the string:\n");
    DumpString(_lstring);
  }
}

void LEngine::DeriveForward(int tbl) {
  ASSERT(ValidLsystem());

  Debug("Deriving forward\n");
  LstringIterator iterator(_lstring);
  _derivedstring.Clear();
  const bool ring = _dll.RingLsystem();

  if (comlineparam.DebugMode()) {
    Utils::Message("Dumping the string:\n");
    DumpString(_lstring);
  }

  __lc_CallerData cd;
  if (!iterator.AtEnd())
    for (;;) {
      // if the current modeul is Cut
      if (Cut_id == iterator.GetModuleId()) {
        // skip to the end of the current branch
        // or the end of the string
        if (!iterator.FindEOB())
          break;
      } else {
        bool applied = false;
        Debug("Looking for production for %s --> ",
              GetNameOf(iterator.GetModuleId()));
        int nop = _dll.NumOfModulePProductions(tbl, iterator.GetModuleId());
        int nop0 = _dll.NumOfModulePProductions(0, iterator.GetModuleId());
        // first check the current group/table
        if (0 != tbl && nop > 0)
          applied = TryForwardGroup(tbl, iterator, cd, nop, ring);
        // then check the default group/table
        if (!applied && nop0 > 0)
          applied = TryForwardGroup(0, iterator, cd, nop0, ring);

        // if a production fired
        if (applied) {
          Debug("applied\n");
          // append the contents of the successor storage
          // to the new string
          _derivedstring.Add(Interface::GetSuccessorStorage());
          // advance the iterator
          // by the number of modules
          // in the strict predecessor
          if (_dll.RingLsystem()) {
            if (!(iterator.TryAdvance(cd.Strct().Count())))
              break;
          } else if (!(iterator += cd.Strct().Count()))
            break;
        }
        // otherwise
        else {
          // apply identity production
          Debug("identity applied\n");
          _derivedstring.Append(iterator);
          // and advance
          if (!++iterator)
            break;
        }
      }
    }
  // swap the new string with the old one
  _lstring.Swap(_derivedstring);
  if (comlineparam.DebugMode() || comlineparam.DumpString()) {
    Utils::Message("After deriving forward\n");
    DumpString(_lstring);
  }
}

void LEngine::DeriveGForward(int tbl) {
  ASSERT(ValidLsystem());

  Debug("Deriving Gillespie forward\n");
  LstringIterator iterator(_lstring);
  _derivedstring.Clear(eForward);
  const bool ring = _dll.RingLsystem();

  __lc_CallerData cd;
  std::vector<GillespieSuccessor> gsa;
  typedef std::vector<GillespieSuccessor>::const_iterator iter;
  // iterate the string
  if (!iterator.AtEnd())
    for (;;) {
      // skip the branch if module Cut found
      if (Cut_id == iterator.GetModuleId()) {
        Debug("Skipping module %s\n", iterator.GetModuleName());
        if (!iterator.FindEOB())
          break;
      } else {
        // Apply productions from the current group to the current module
        Debug("Applying Gillespie group %d to module %s\n", tbl,
              iterator.GetModuleName());
        int nop = _dll.NumOfModulePProductions(tbl, iterator.GetModuleId());
        ApplyForwardGGroup(tbl, iterator, cd, nop, gsa, ring);
      }
      if (!++iterator)
        break;
    }

  // calculate the sum of propensities
  double sum = 0.0;
  {
    for (iter it = gsa.begin(); it != gsa.end(); ++it)
      sum += it->Propensity();
  }

  Debug("Sum of propensities is %g\n", sum);

  if (sum > 0.0) {
    // get a random number in the range [0, sum)
    double rnd = sum * GRand();
    Debug("Random number is %g\n", rnd);

    // iterate the array of Gillespie successors
    for (iter it = gsa.begin(); it != gsa.end(); ++it) {
      rnd -= it->Propensity();
      // found the successor
      if (rnd <= 0.0) {
        Debug("Replacing\n");
        // replace the predecessor with successor directly in the main string
        _lstring.Replace(_derivedstring, it->Successor(), it->Predecessor());
        break;
      }
    }
  }

  if (comlineparam.DebugMode() || comlineparam.DumpString()) {
    Utils::Message("After deriving forward\n");
    DumpString(_lstring);
  }

  // advance time
  AdvanceGillespieTime(sum);
}

void LEngine::DeriveGBackward(int) {
  Utils::Message("Deriving backward Gillespie: not implemented yet!");
}

void LEngine::AdvanceGillespieTime(double pr) {
  if (pr > 0.0)
    _GillespieTime -= 1.0 / pr * log(1.0 - GRand());
}

void LEngine::ApplyForwardGGroup(int tbl, const LstringIterator &iter,
                                 __lc_CallerData &cd, int nop,
                                 std::vector<GillespieSuccessor> &gsa,
                                 bool ring) {
  // for every production in the group
  for (int i = 0; i < nop; ++i) {
    cd.Reset();
    // get the predecessor
    const __lc_ProductionPredecessor &pred =
        _dll.GetModulePProductionPredecessor(tbl, iter.GetModuleId(), i);
    // check if matches the current position
    ProductionMatchIteratorSet iteratorSet(_lstring, _derivedstring);
    __lc_ProdCaller pCaller =
        TryMatchForward(iter, cd, pred, iteratorSet, ring);
    // if it does
    if (0 != pCaller) {
      GillespieSuccessor gs;
      // store the start and end position of the predecessor modules
      // for the current production
      {
        Lstring::Range rng;
        rng.Begin(iter.Position());
        LstringIterator ti(iter);
        ti += pred.Strct.count;
        rng.End(ti.Position());
        gs.Predecessor(rng);
      }

      Interface::GetSuccessorStorage().Clear();
      // run the production
      pCaller(&cd);
      Lstring tmp(&_pool);
      // put the successor into a temporary string
      tmp.Add(Interface::GetSuccessorStorage());
      LstringIterator i = tmp;
      // if empty
      if (i.AtEnd())
        // forget it
        break;
      // if first module is not Propensity
      if (i.GetModuleId() != Propensity_id) {
        // complain and break
        Utils::Message(
            "Invalid successor for Gillespie production for module %s\n",
            iter.GetModuleName());
        break;
      }
      // otherwise
      for (; !i.AtEnd(); ++i) {
        // if current module is Propensity
        if (i.GetModuleId() == Propensity_id) {
          // it is not the first successor generated
          // by the production
          if (!i.AtBeginning()) {
            Lstring::Range end = gs.Successor();
            // then set the end position
            // of the previous successor
            end.End(i.Position() + _derivedstring.BytesUsed());
            gs.Successor(end);
            // and add the information to the array
            gsa.push_back(gs);
          }
          const __lc_BasicParameterStruct *pBasicParams =
              reinterpret_cast<const __lc_BasicParameterStruct *>(i.Ptr());
          const __Propensity_ParamsStruct_ *pParams =
              reinterpret_cast<const __Propensity_ParamsStruct_ *>(
                  pBasicParams);
          const __Propensity_ParamsStruct_::Data &params = pParams->data;
          float v = params.Param0;
          if (v >= 0.0f) {
            // report problems
#ifdef WIN32
            if (!_finite(v))
#else
            if (!std::isfinite(v))
#endif // WIN32
            {
              Utils::Message("Warning: nonfinite propensity found - check for "
                             "divison by zero\n");
              gs.Propensity(0.0f);
            } else
              gs.Propensity(v);
          } else {
            Utils::Message("Propensity should be > 0. Assuming zero\n");
            gs.Propensity(0.0f);
          }

          // set the beginning of the successor
          // to the module following Propensity (the current one)
          {
            Lstring::Range end = gs.Successor();
            end.Begin(i.Position() + i.GetModuleSize() +
                      _derivedstring.BytesUsed());
            gs.Successor(end);
          }
        }
      }
      // end of the successors generated by the current production
      if (!i.AtBeginning()) {
        // add the end position
        // for the last successor
        Lstring::Range end = gs.Successor();
        end.End(i.Position() + _derivedstring.BytesUsed());
        gs.Successor(end);
        gsa.push_back(gs);
      }
      // append the successors
      // from the current production
      // to _derivedstring
      _derivedstring.Append(tmp);
    }
  }
}

bool LEngine::TryForwardGroup(int grp, LstringIterator &iterator,
                              __lc_CallerData &cd, int nop, bool ring) {
  // iterate through the productions
  // in the group grp
  for (int i = 0; i < nop; ++i) {
    // first reset the CallerData
    cd.Reset();
    // get the predecessor for the current production
    const __lc_ProductionPredecessor &pred =
        _dll.GetModulePProductionPredecessor(grp, iterator.GetModuleId(), i);
    // check if it matches the current string position

    ProductionMatchIteratorSet iteratorSet(_lstring, _derivedstring);
    __lc_ProdCaller pCaller =
        TryMatchForward(iterator, cd, pred, iteratorSet, ring);

    // if it does then execute
    if (0 != pCaller) {
      // clear the successor storage
      Interface::GetSuccessorStorage().Clear();
      // and try to apply the production
      RightContextScanner rContextScanner(*this, iteratorSet.rightContext, ring,
                                          pred.iConsiderGroup);
      pgRContextScanner = &rContextScanner;
      LeftContextScanner lContextScanner(*this, iteratorSet.leftContext, ring,
                                         pred.iConsiderGroup);
      pgLContextScanner = &lContextScanner;
      pgRNContextScanner = NULL;
      LeftContextScanner lnContextScanner(*this, iteratorSet.leftNewContext,
                                          false, pred.iConsiderGroup);
      pgLNContextScanner = &lnContextScanner;
      bool bApplied = pCaller(&cd);
      pgRContextScanner = NULL;
      pgLContextScanner = NULL;
      pgRNContextScanner = NULL;
      pgLNContextScanner = NULL;
      if (bApplied)
        return true;
    }
  }
  // no production applied
  return false;
}

void LEngine::DeriveBackward(int tbl) {
  ASSERT(ValidLsystem());
  Debug("Deriving backward\n");

  LstringIterator iterator(_lstring);
  // the new string grows backward
  _derivedstring.Clear(eBackward);
  // first go to the end of the string
  iterator.FindEOS();

  const bool ring = _dll.RingLsystem();

  __lc_CallerData cd;
  // if not at the beginning
  while (!iterator.AtBeginning()) {
    // move one position back
    --iterator;
    // warn about Cut module in the string
    // cut is not performed when deriving backward
    if (Cut_id == iterator.GetModuleId())
      Utils::Message("Cut module found while deriving backwards. Ignored.\n");

    bool applied = false;
    int nop = _dll.NumOfModulePProductions(tbl, iterator.GetModuleId());
    Debug("Looking for production for %s --> ",
          GetNameOf(iterator.GetModuleId()));
    // first try the current group/table
    if (0 != tbl && nop > 0)
      applied = TryBackwardGroup(tbl, iterator, cd, nop, ring);
    // if nothing applied
    // then try the default group/table
    int nop0 = _dll.NumOfModulePProductions(0, iterator.GetModuleId());
    if (!applied && nop0 > 0)
      applied = TryBackwardGroup(0, iterator, cd, nop0, ring);
    // if no production applied
    if (!applied) {
      Debug("identity applied\n");
      Interface::GetSuccessorStorage().Clear();
      // apply identity production
      iterator.AppendCurrent(Interface::GetSuccessorStorage());
    }
    // add the contents of the successor
    // storage to the new string
    _derivedstring.Add(Interface::GetSuccessorStorage());
  }
  // substitute the new string for the old one
  _lstring.Swap(_derivedstring);
  if (comlineparam.DebugMode() || comlineparam.DumpString()) {
    Utils::Message("After deriving backward\n");
    DumpString(_lstring);
  }
}

__lc_ProdCaller
LEngine::TryMatchForward(const LstringIterator &citer, __lc_CallerData &cd,
                         const __lc_ProductionPredecessor &pred,
                         ProductionMatchIteratorSet &iteratorSet,
                         bool bRing) const {
  TIME_THIS(1);
  ASSERT(ValidLsystem());
  // when deriving forward
  // we ignore productions
  // that have new right context
  if (pred.RCntxt.HasNewContext())
    return 0;
  LstringIterator iter(citer);
  LIteratorReset reset(iter);
  // first check if the strict predecessor matches
  if (!CheckStrictPredecessor(iter, cd, pred.Strct, bRing))
    return 0;
  // then check the right predecessor
  if (!CheckRightContext(iter, cd, pred.RCntxt.Cntxt, bRing,
                         pred.iConsiderGroup))
    return 0;
  iteratorSet.rightContext = iter;
  // check the new left context if required
  LstringIterator diter(_derivedstring);
  diter.FindEOS();

  // return to the beginning of the strict predecessor
  reset.Reset();
  if (pred.LCntxt.HasNewContext()) {
    if (!CheckLeftContext(diter, cd, pred.LCntxt.NewCntxt, false,
                          pred.iConsiderGroup))
      return 0;
  }
  // or the left context
  else {
    if (!CheckLeftContext(iter, cd, pred.LCntxt.Cntxt, bRing,
                          pred.iConsiderGroup))
      return 0;
  }
  iteratorSet.leftContext = iter;
  iteratorSet.leftNewContext = diter;
  // matching production found
  // return address of the caller
  return pred.pCaller;
}

bool LEngine::CheckStrictPredecessor(LstringIterator &iter, __lc_CallerData &cd,
                                     const __lc_ProductionModules &proto,
                                     bool bRing) const {
  ASSERT(proto.count > 0);
  // compare the current module in the string
  // and the module in the predecessor
  if (iter.GetModuleId() != proto.arr[0]) {
    return false;
  }

  // if they match
  // add the parameter address in the string
  cd.Strct().AddModuleAddr(iter.Ptr());

  // continue for the remaining
  // modules in the strict predecessor
  if (1 < proto.count) {
    for (int i = 1; i < proto.count; ++i) {
      // break if end of string
      if (!++iter) {
        if (bRing)
          iter.FindBOS();
        else
          return false;
      }
      // break if module in the string
      // doesn't match the module in the strict predecessor
      if (iter.GetModuleId() != proto.arr[i])
        return false;
      // otherwise add the parameter address
      // in the string
      cd.Strct().AddModuleAddr(iter.Ptr());
    }
  }
  // when leaving this function
  // the iterator points to the location
  // past the strict predecessor
  ++iter;
  return true;
}

bool LEngine::CheckRightContext(LstringIterator &iter, __lc_CallerData &cd,
                                const __lc_ProductionModules &proto, bool bRing,
                                int iConsiderGroup) const {
  RightContextScanner scanner(*this, iter, bRing, iConsiderGroup);
  scanner.Reset();
  // for every module in
  // the production right context
  for (int i = 0; i < proto.count;) {
    if (!scanner.Accept(proto.arr[i])) {
      return false;
    }
    // otherwise matching module found
    else {
      // so add its address
      cd.RCntxt().AddModuleAddr(scanner.Address());
      ++i;
      scanner.Advance();
    }
  }
  // match
  return true;
}

bool LEngine::CheckLeftContext(LstringIterator &iter, __lc_CallerData &cd,
                               const __lc_ProductionModules &proto, bool bRing,
                               int iConsiderGroup) const {
  // iterate backward the module in the context
  // and in the string
  LeftContextScanner scanner(*this, iter, bRing, iConsiderGroup);
  scanner.Reset();

  for (int i = proto.count - 1; i >= 0; --i) {
    if (!scanner.Accept(proto.arr[i]))
      return false;

    // otherwise add the address of the module
    cd.LCntxt().AddModuleAddr(iter.Ptr());
    scanner.Advance();
  }
  // because the modules in the left context
  // are found in the reverse order
  // (we match them going from right to left)
  // the returned data has to be reversed
  cd.LCntxt().Reverse();
  return true;
}

bool LEngine::FindMatch(const LstringIterator &citer, __lc_CallerData &cd,
                        const MatchParameters &matchParameters) const {
  ASSERT(ValidLsystem());
  // lets look through and try and find one that matches this module
  Debug("trying dmatch\n");
  LstringIterator iter(citer);
  LIteratorReset reset(iter);
  int iNumberOfProductions =
      matchParameters.NumOfModuleProductions(citer.GetModuleId());
  for (int i = 0; i < iNumberOfProductions; ++i) {
    reset.Reset();
    const __lc_ProductionPredecessor &pred =
        matchParameters.GetModuleProductionPredecessor(citer.GetModuleId(), i);

    Debug("pred.Strct[0](%d)\n", pred.Strct.arr[0]);
    // when deriving forward
    // we ignore productions
    // that have new right context
    if (pred.RCntxt.HasNewContext()) {
      Debug("production has new right context\n");
      continue;
    }
    // first check if the strict predecessor matches
    if (!CheckStrictPredecessor(iter, cd, pred.Strct,
                                matchParameters.IsRing())) {
      Debug("no SP match\n");
      continue;
    }
    // then check the right predecessor
    if (!CheckRightContext(iter, cd, pred.RCntxt.Cntxt,
                           matchParameters.IsRing(), pred.iConsiderGroup)) {
      Debug("no RC match\n");
      continue;
    }
    // check the new left context if required
    if (pred.LCntxt.HasNewContext()) {
      Debug("has new left context, checking");
      LstringIterator diter(_derivedstring);
      diter.FindEOS();
      if (!CheckLeftContext(diter, cd, pred.LCntxt.NewCntxt, false,
                            pred.iConsiderGroup))
        continue;
    }
    // or the left context
    else {
      // return to the beginning of the strict predecessor
      reset.Reset();
      if (!CheckLeftContext(iter, cd, pred.LCntxt.Cntxt,
                            matchParameters.IsRing(), pred.iConsiderGroup)) {
        Debug("no LC match");
        continue;
      }
    }
    // matching production found
    // return address of the caller

    Debug("matched module %s\n", GetNameOf(iter.GetModuleId()));
    // return pred.pCaller;
    Interface::GetSuccessorStorage().Clear();
    if (pred.pCaller(&cd)) {
      return true;
    }
  }
  return false;
}

#define USE_CONSIDER_LOOKUP

bool LEngine::Considered(__lc_ModuleIdType mid, int iConsiderGroup) const {
  // specifies whether the given module
  // should be considered
#ifdef USE_CONSIDER_LOOKUP
  ASSERT(ValidLsystem());
  return _dll.IsConsidered(mid, iConsiderGroup);
#else
  if (SB_id == mid)
    return true;
  if (EB_id == mid)
    return true;
  if (_dll.NumOfConsidered() > 0)
    return InConsidered(mid);
  if (_dll.NumOfIgnored() > 0)
    return !InIgnored(mid);
  return true;
#endif
}

bool LEngine::TryBackwardGroup(int grp, LstringIterator &iterator,
                               __lc_CallerData &cd, int nop, bool ring) {
  // iterate the grp group
  // of productions
  for (int i = 0; i < nop; ++i) {
    cd.Reset();
    // get the production predecessor
    const __lc_ProductionPredecessor &pred =
        _dll.GetModulePProductionPredecessor(grp, iterator.GetModuleId(), i);
    // check if it matches the current
    // position in the string

    ProductionMatchIteratorSet iteratorSet(_lstring, _derivedstring);
    __lc_ProdCaller pCaller =
        TryMatchBackward(iterator, cd, pred, iteratorSet, ring);
    // if a matching production is found
    if (0 != pCaller) {
      // clear the successor storage
      Interface::GetSuccessorStorage().Clear();
      // and try to apply
      RightContextScanner rContextScanner(*this, iteratorSet.rightContext, ring,
                                          pred.iConsiderGroup);
      pgRContextScanner = &rContextScanner;
      LeftContextScanner lContextScanner(*this, iteratorSet.leftContext, ring,
                                         pred.iConsiderGroup);
      pgLContextScanner = &lContextScanner;
      RightContextScanner rnContextScanner(*this, iteratorSet.rightNewContext,
                                           false, pred.iConsiderGroup);
      pgRNContextScanner = &rnContextScanner;
      pgLNContextScanner = NULL;
      bool bApplied = pCaller(&cd);
      pgRContextScanner = NULL;
      pgLContextScanner = NULL;
      pgRNContextScanner = NULL;
      pgLNContextScanner = NULL;
      if (bApplied) {
        Debug("applied\n");
        // if applied move the iterator
        // backward by the number of modules
        // in the applied production's
        // strict predecessor
        if (pred.Strct.count > 1)
          iterator -= pred.Strct.count - 1;
        // end exit
        return true;
      }
    }
  }
  // no production applied
  return false;
}

__lc_ProdCaller
LEngine::TryMatchBackward(const LstringIterator &citer, __lc_CallerData &cd,
                          const __lc_ProductionPredecessor &pred,
                          ProductionMatchIteratorSet &iteratorSet,
                          bool bRing) const {
  TIME_THIS(1);
  ASSERT(ValidLsystem());
  // when deriving backward
  // we ignore productions
  // that have new left context
  if (pred.LCntxt.HasNewContext())
    return 0;
  LstringIterator iter(citer);
  // try to move back
  // by the number of modules
  // in the strict predecessor
  if (pred.Strct.count > 1) {
    for (int iModule = 1; iModule < pred.Strct.count; ++iModule) {
      if (!iter.Back(bRing))
        return NULL;
    }
  }

  LIteratorReset reset(iter);
  // first check the strict predecessor
  if (!CheckStrictPredecessor(iter, cd, pred.Strct, bRing))
    return 0;
  // if new right context is present
  // then check for match
  LstringIterator diter(_derivedstring);
  if (pred.RCntxt.HasNewContext()) {
    if (!CheckRightContext(diter, cd, pred.RCntxt.NewCntxt, false,
                           pred.iConsiderGroup))
      return 0;
  }
  // otherwise
  else {
    // check the right context
    if (!CheckRightContext(iter, cd, pred.RCntxt.Cntxt, bRing,
                           pred.iConsiderGroup))
      return 0;
  }
  iteratorSet.rightNewContext = diter;
  iteratorSet.rightContext = iter;
  // return to the beginning of the strict predecessor
  reset.Reset();
  // and check the left context
  if (!CheckLeftContext(iter, cd, pred.LCntxt.Cntxt, bRing,
                        pred.iConsiderGroup))
    return 0;
  iteratorSet.leftContext = iter;
  // if everything matches
  // return the production caller
  return pred.pCaller;
}
