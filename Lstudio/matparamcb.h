#ifndef __MATPARAMCB_H__
#define __MATPARAMCB_H__


class MaterialEdit;

class MaterialParamCallback : public ColorSliderCallback
{
public:
	enum ParameterType
	{
		eAmbient,
			eDiffuse,
			eEmission,
			eSpecular,
			eShininess,
			eTransparency
	};
	MaterialParamCallback(MaterialEdit* pEdit, ParameterType parameter) : 
	  _pEdit(pEdit),
		  _parameter(parameter)
	{}
	void ColorChanged(COLORREF, bool);
private:
	MaterialEdit* _pEdit;
	const ParameterType _parameter;
};




#else
	#error File already included
#endif