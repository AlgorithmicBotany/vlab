#include <fw.h>

#include "user.h"

#include <browser/remaccess.h>
#include <browser/labtbl.h>
#include <browser/vlabbrowser.h>
#include <browser/resids.h>

SharedUser::SharedUser()
{
	VLB::VlabBrowser::Register(App::GetInstance(), IDI_BRICON);
}


SharedUser::~SharedUser()
{}
