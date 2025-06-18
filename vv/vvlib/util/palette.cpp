#include "palette.hpp"
#include <fstream>

/** @brief Constructor.
    @param filename The palette file to load.
*/
util::Palette::Palette(std::string filename) :
  filename(filename)
{
  reread();
}

/** @brief A function to reread the palette file from disc.
 */
void util::Palette::reread() {
  std::ifstream in(filename.c_str(), std::ios::binary);

  unsigned int index = 0;
  while (!in.eof() || !in.good() || !in) {
    if (index > 255) break;
    colours[index].r(in.get ());
    colours[index].g(in.get ());
    colours[index].b(in.get ());
    index++;
  }
}

/** @brief Sets the OpenGL colour to that selected from the palette.
    @param index The palette index.

    If index is larger than 255 it is set to 255.
*/
void util::Palette::useColour(unsigned int index, double alpha) {
  if (index > 255) index = 255;
  glColor4ub(colours[index].r(), colours[ index ].g(), colours[ index ].b(),
              GLubyte( alpha*255 ) );
}

/** @brief Returns the selected colour from the palette.
    @param index The palette index.

    If index is larger than 255 it is set to 255.
*/
const util::Palette::PaletteColour& util::Palette::getColour(unsigned int index) {
  if (index > 255) index = 255;
  return colours[index];
}

/** @brief Sets the OpenGL colour to the average of the two selected
           colours.
    @param ind1 The first selected colour.
    @param ind2 The second selected colour.
    @param w    The weight given to the first colour.  The second
                colour is weighted as 1-w.

    If either ind1 or ind2 are larger than 255, then they are set to
    255.  Since the colours are stored as unsigned bytes, integer
    truncation may occur during the averaging.
*/
void util::Palette::blend(unsigned int ind1, unsigned int ind2, double w, double alpha ) {
  if (ind1 > 255) ind1 = 255;
  if (ind2 > 255) ind2 = 255;
  if (w > 1.0) w = 1.0;
  if (w < 0.0) w = 0.0;

  if (ind1 == ind2) glColor3ubv(colours[ind1].c_data());
  else {
    PaletteColour a = colours[ind1], b = colours[ind2];
    glColor4ub(
      GLubyte(a.r() * w + b.r() * (1.0 - w)),
      GLubyte(a.g() * w + b.g() * (1.0 - w)),
      GLubyte(a.b() * w + b.b() * (1.0 - w)),
      GLubyte(alpha*255)
    );
  }
}
