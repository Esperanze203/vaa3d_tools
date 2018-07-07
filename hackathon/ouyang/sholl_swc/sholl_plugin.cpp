/*
 *  sholl_func.cpp
 *  io functions for domenu and do func
 *  2018-06-26: by OuYang Qiang
 */
 
#include "v3d_message.h"

#include "sholl_plugin.h"
#include "sholl_func.h"
 
Q_EXPORT_PLUGIN2(sholl_swc, SHOLLPlugin);
 
QStringList SHOLLPlugin::menulist() const
{
	return QStringList()
        <<tr("sholl_swc")
		<<tr("about");
}

QStringList SHOLLPlugin::funclist() const
{
	return QStringList()
        <<tr("sholl_swc")
		<<tr("help");
}

void SHOLLPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("sholl_swc"))
	{
        sholl_menu(callback,parent);
	}
	else if (menu_name == tr("help"))
	{
		printHelp(callback,parent);
	}
	else
	{
		printHelp(callback,parent);
	}
}

bool SHOLLPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("sholl_swc"))
	{
        sholl_func(input, output);
	  return true;
	}
	else if (func_name == tr("help"))
	{
		printHelp(input,output);
	  return true;
	}
    else if (func_name == tr("TOOLBOXsholl_swc"))
	{
        sholl_toolbox(input);
		return true;
	}
	return false;
}

