#include <fw.h>

#include <gl\gl.h>

#include "materialparams.h"
#include "liboglstrng.h"

MaterialParams::MaterialParams()
{
	Reset();
}

void MaterialParams::Reset()
{
	{
		_ambient[0] = 0.0f;
		_ambient[1] = 0.0f;
		_ambient[2] = 0.0f;
		_ambient[3] = 1.0f;
	}
	{
		_diffuse[0] = 0.8f;
		_diffuse[1] = 0.8f;
		_diffuse[2] = 0.8f;
		_diffuse[3] = 1.0f;
	}
	{
		_specular[0] = 0.0f;
		_specular[1] = 0.0f;
		_specular[2] = 0.0f;
		_specular[3] = 1.0f;
	}
	{
		_emission[0] = 0.0f;
		_emission[1] = 0.0f;
		_emission[2] = 0.0f;
		_emission[3] = 1.0f;
	}
	_shininess = 0;
}


void MaterialParams::Apply() const
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, _ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, _diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, _specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, _emission);
	glMateriali(GL_FRONT, GL_SHININESS, _shininess);
}


void MaterialParams::Write(WriteBinFile& trg) const
{
	BYTE bf[17];
	bf[0] = (BYTE)(_ambient[0] * 256);
	bf[1] = (BYTE)(_ambient[1] * 256);
	bf[2] = (BYTE)(_ambient[2] * 256);
	bf[3] = (BYTE)(_ambient[3] * 256);
	bf[4] = (BYTE)(_diffuse[0] * 256);
	bf[5] = (BYTE)(_diffuse[1] * 256);
	bf[6] = (BYTE)(_diffuse[2] * 256);
	bf[7] = (BYTE)(_diffuse[3] * 256);
	bf[8] = (BYTE)(_specular[0] * 256);
	bf[9] = (BYTE)(_specular[1] * 256);
	bf[10] = (BYTE)(_specular[2] * 256);
	bf[11] = (BYTE)(_specular[3] * 256);
	bf[12] = (BYTE)(_emission[0] * 256);
	bf[13] = (BYTE)(_emission[1] * 256);
	bf[14] = (BYTE)(_emission[2] * 256);
	bf[15] = (BYTE)(_emission[3] * 256);
	bf[16] = (BYTE)_shininess;
	trg.Write(bf, sizeof(BYTE) * 17);
}

void MaterialParams::Write(WriteTextFile& trg) const
{
	trg.PrintF("A %f %f %f %f\n", _ambient[0], _ambient[1], _ambient[2], _ambient[3]);
	trg.PrintF("D %f %f %f %f\n", _diffuse[0], _diffuse[1], _diffuse[2], _diffuse[3]);
	trg.PrintF("S %f %f %f %f\n", _specular[0], _specular[1], _specular[2], _specular[3]);
	trg.PrintF("E %f %f %f %f\n", _emission[0], _emission[1], _emission[2], _emission[3]);
	trg.PrintF("Sh %d\n", _shininess);
}


void MaterialParams::Load(ReadTextFile& src)
{
	std::string line;
	float readarr[4];
	src.Read(line);
	if (4 != sscanf
		(
		line.c_str(), 
		"A %f %f %f %f\n", 
		&(readarr[0]), 
		&(readarr[1]), 
		&(readarr[2]), 
		&(readarr[3])
		)
		)
		throw Exception(GetLibOglString(FWOGLStr::ReadingMaterial), src.Filename());
	_ambient[0] = readarr[0];
	_ambient[1] = readarr[1];
	_ambient[2] = readarr[2];
	_ambient[3] = readarr[3];
	src.Read(line);
	if (4 != sscanf
		(
		line.c_str(), 
		"D %f %f %f %f\n", 
		&(readarr[0]), 
		&(readarr[1]), 
		&(readarr[2]), 
		&(readarr[3])
		)
		)
		throw Exception(GetLibOglString(FWOGLStr::ReadingMaterial), src.Filename());
	_diffuse[0] = readarr[0];
	_diffuse[1] = readarr[1];
	_diffuse[2] = readarr[2];
	_diffuse[3] = readarr[3];
	src.Read(line);
	if (4 != sscanf
		(
		line.c_str(), 
		"S %f %f %f %f\n", 
		&(readarr[0]), 
		&(readarr[1]), 
		&(readarr[2]), 
		&(readarr[3])
		)
		)
		throw Exception(GetLibOglString(FWOGLStr::ReadingMaterial), src.Filename());
	_specular[0] = readarr[0];
	_specular[1] = readarr[1];
	_specular[2] = readarr[2];
	_specular[3] = readarr[3];
	src.Read(line);
	if (4 != sscanf
		(
		line.c_str(), 
		"E %f %f %f %f\n", 
		&(readarr[0]), 
		&(readarr[1]), 
		&(readarr[2]), 
		&(readarr[3])
		)
		)
		throw Exception(GetLibOglString(FWOGLStr::ReadingMaterial), src.Filename());
	_emission[0] = readarr[0];
	_emission[1] = readarr[1];
	_emission[2] = readarr[2];
	_emission[3] = readarr[3];
	src.Read(line);
	if (1 != sscanf(line.c_str(), "Sh %d\n", &_shininess))
		throw Exception(GetLibOglString(FWOGLStr::ReadingMaterial), src.Filename());
}

void MaterialParams::Load(ReadBinFile& src)
{
	BYTE bf[17];
	src.Read(bf, sizeof(BYTE) * 17);
	_ambient[0] = bf[0] / 256.0f;
	_ambient[1] = bf[1] / 256.0f;
	_ambient[2] = bf[2] / 256.0f;
	_ambient[3] = bf[3] / 256.0f;
	_diffuse[0] = bf[4] / 256.0f;
	_diffuse[1] = bf[5] / 256.0f;
	_diffuse[2] = bf[6] / 256.0f;
	_diffuse[3] = bf[7] / 256.0f;
	_specular[0] = bf[8] / 256.0f;
	_specular[1] = bf[9] / 256.0f;
	_specular[2] = bf[10] / 256.0f;
	_specular[3] = bf[11] / 256.0f;
	_emission[0] = bf[12] / 256.0f;
	_emission[1] = bf[13] / 256.0f;
	_emission[2] = bf[14] / 256.0f;
	_emission[3] = bf[15] / 256.0f;
	_shininess = bf[16];
}

bool MaterialParams::operator!=(const MaterialParams& r) const
{
	if (_ambient[0] != r._ambient[0])
		return true;
	if (_ambient[1] != r._ambient[1])
		return true;
	if (_ambient[2] != r._ambient[2])
		return true;
	if (_ambient[3] != r._ambient[3])
		return true;

	if (_diffuse[0] != r._diffuse[0])
		return true;
	if (_diffuse[1] != r._diffuse[1])
		return true;
	if (_diffuse[2] != r._diffuse[2])
		return true;
	if (_diffuse[3] != r._diffuse[3])
		return true;

	if (_specular[0] != r._specular[0])
		return true;
	if (_specular[1] != r._specular[1])
		return true;
	if (_specular[2] != r._specular[2])
		return true;
	if (_specular[3] != r._specular[3])
		return true;

	if (_emission[0] != r._emission[0])
		return true;
	if (_emission[1] != r._emission[1])
		return true;
	if (_emission[2] != r._emission[2])
		return true;
	if (_emission[3] != r._emission[3])
		return true;

	if (_shininess != r._shininess)
		return true;

	return false;
}

