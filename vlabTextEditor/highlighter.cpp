/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



// Based on Qt Example

#include <QtGui>

#include "highlighter.h"

//! [0]
Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  keywordFormat.setForeground(Qt::yellow);
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;
  keywordPatterns << "\\bclass\\b"
                  << "\\bconst\\b"
                  << "\\bfriend\\b"
                  << "\\bnamespace\\b"
                  << "\\boperator\\b"
                  << "\\bprivate\\b"
                  << "\\bprotected\\b"
                  << "\\bpublic\\b"
                  << "\\bsignals\\b"
                  << "\\bslots\\b"
                  << "\\bstatic\\b"
                  << "\\btemplate\\b"
                  << "\\bvirtual\\b"
                  << "\\bfor\\b"
                  << "\\bif\\b"
                  << "\\+"
                  << "\\-"
                  << "\\*"
                  << "\\/";
  foreach (const QString &pattern, keywordPatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
    //! [0] //! [1]
  }
  //! [1]

  keywordTypeFormat.setForeground(Qt::green);
  keywordTypeFormat.setFontWeight(QFont::Bold);
  QStringList keywordTypePatterns;
  keywordTypePatterns << "\\bchar\\b"
                      << "\\bconst\\b"
                      << "\\bdouble\\b"
                      << "\\benum\\b"
                      << "\\bexplicit\\b"
                      << "\\bfriend\\b"
                      << "\\binline\\b"
                      << "\\bint\\b"
                      << "\\blong\\b"
                      << "\\bshort\\b"
                      << "\\bsigned\\b"
                      << "\\bstatic\\b"
                      << "\\bstruct\\b"
                      << "\\btypedef\\b"
                      << "\\btypename\\b"
                      << "\\bunion\\b"
                      << "\\bunsigned\\b"
                      << "\\bvoid\\b"
                      << "\\bvolatile\\b";
  foreach (const QString &pattern, keywordTypePatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = keywordTypeFormat;
    highlightingRules.append(rule);
    //! [0] //! [1]
  }

  //! [2]
  lpfgKeyWordFormat.setFontWeight(QFont::Bold);
  lpfgKeyWordFormat.setForeground(Qt::yellow);
  QStringList lpfgKeywordPatterns;
  lpfgKeywordPatterns << "\\bAxiom\\b"
                      << "\\bderivation length\\b"
                      << "\\bmodule\\b"
                      << "\\bStart\\b"
                      << "\\bEnd\\b"
                      << "\\bStartEach\\b"
                      << "\\bEndEach\\b"
                      << "\\bdecomposition\\b"
                      << "\\bmaximum depth\\b"
                      << "\\bhomomorphism\\b"
                      << "\\bendlsystem\\b"
                      << "\\bfor\\b";
  foreach (const QString &pattern, lpfgKeywordPatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = lpfgKeyWordFormat;
    highlightingRules.append(rule);
    //! [0] //! [1]
  }

  //! [2]
  lpfgProduceKeyWordFormat.setFontWeight(QFont::Bold);
  lpfgProduceKeyWordFormat.setForeground(Qt::cyan);
  QStringList lpfgProduceKeywordPatterns;
  lpfgProduceKeywordPatterns << "\\bproduce\\b";
  foreach (const QString &pattern, lpfgProduceKeywordPatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = lpfgProduceKeyWordFormat;
    highlightingRules.append(rule);
    //! [0] //! [1]
  }

  rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
  rule.format = classFormat;
  highlightingRules.append(rule);
  //! [2]

  //! [2]
  classFormat.setFontWeight(QFont::Bold);
  classFormat.setForeground(Qt::green);
  rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
  rule.format = classFormat;
  highlightingRules.append(rule);
  //! [2]

  //! [4]
  quotationFormat.setForeground(Qt::darkGreen);
  rule.pattern = QRegExp("\".*\"");
  rule.format = quotationFormat;
  highlightingRules.append(rule);
  //! [4]

  //! [5]
  functionFormat.setFontItalic(false);
  functionFormat.setForeground(Qt::green);
  rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
  rule.format = functionFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  definesFormat.setFontItalic(false);
  definesFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#define[^\n]*");
  rule.format = definesFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  definesFormat.setFontItalic(false);
  definesFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#ifdef[^\n]*");
  rule.format = definesFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  definesFormat.setFontItalic(false);
  definesFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#if[^\n]*");
  rule.format = definesFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  definesFormat.setFontItalic(false);
  definesFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#endif");
  rule.format = definesFormat;
  highlightingRules.append(rule);
  //! [5]
  //! [5]
  definesFormat.setFontItalic(false);
  definesFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#else");
  rule.format = definesFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  includeFormat.setFontItalic(false);
  includeFormat.setForeground(Qt::magenta);
  rule.pattern = QRegExp("#include");
  rule.format = includeFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  numberFormat.setFontItalic(false);
  numberFormat.setForeground(QColor(255, 150, 100));
  rule.pattern = QRegExp("\\b[-+]?[0-9]+\\b");
  rule.format = numberFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [5]
  includePathFormat.setFontItalic(false);
  includePathFormat.setForeground(QColor(255, 150, 100));
  rule.pattern = QRegExp("<[^\n]*>");
  rule.format = includePathFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [3]
  singleLineCommentFormat.setForeground(QColor(120, 120, 200));
  rule.pattern = QRegExp("//[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);

  multiLineCommentFormat.setForeground(QColor(120, 120, 200));
  //! [3]

  //! [6]
  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");
}
//! [6]

//! [7]
void Highlighter::highlightBlock(const QString &text) {
  foreach (const HighlightingRule &rule, highlightingRules) {
    QRegExp expression(rule.pattern);
    int index = expression.indexIn(text);
    while (index >= 0) {
      int length = expression.matchedLength();
      setFormat(index, length, rule.format);
      index = expression.indexIn(text, index + length);
    }
  }
  //! [7] //! [8]
  setCurrentBlockState(0);
  //! [8]

  //! [9]
  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = commentStartExpression.indexIn(text);

  //! [9] //! [10]
  while (startIndex >= 0) {
    //! [10] //! [11]
    int endIndex = commentEndExpression.indexIn(text, startIndex);
    int commentLength;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength =
          endIndex - startIndex + commentEndExpression.matchedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex =
        commentStartExpression.indexIn(text, startIndex + commentLength);
  }
}
//! [11]
