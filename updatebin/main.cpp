#include <iostream>
#include <QApplication>
#include "platform.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  // run update bin
  Vlab::UpdateBinLog log = Vlab::updateBin();
  // report errors
  if (log.size() > 0) {
    for (size_t i = 0; i < log.size(); i++)
      std::cout << log[i] << "\n";
  }

  return 0;
}
