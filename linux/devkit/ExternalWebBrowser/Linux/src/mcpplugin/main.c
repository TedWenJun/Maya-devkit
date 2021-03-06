//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#import "mcp.h"
#import "PluginObject.h"

NPNetscapeFuncs* browser;

NPError		NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError		NPP_Destroy(NPP instance, NPSavedData** save);
NPError		NPP_SetWindow(NPP instance, NPWindow* window);
NPError		NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
NPError		NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason);
int32		NPP_WriteReady(NPP instance, NPStream* stream);
int32		NPP_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void		NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
void		NPP_Print(NPP instance, NPPrint* platformPrint);
int16		NPP_HandleEvent(NPP instance, void* event);
void		NPP_URLNotify(NPP instance, const char* URL, NPReason reason, void* notifyData);
NPError		NPP_GetValue(NPP instance, NPPVariable variable, void *value);
NPError		NPP_SetValue(NPP instance, NPNVariable variable, void *value);

#define MIME_TYPES_HANDLED "application/x-mcp"
#define PLUGIN_NAME "MCP plug-in"
#define MIME_TYPES_DESCRIPTION MIME_TYPES_HANDLED"::"PLUGIN_NAME
#define PLUGIN_DESCRIPTION "MCP plug-in"

char* NPP_GetMIMEDescription(void)
{
    return MIME_TYPES_DESCRIPTION;
}

NPError NP_GetValue(void *future, NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;
    switch(aVariable) {
        case NPPVpluginNameString:
            *((char **)aValue) = PLUGIN_NAME; break;
        case NPPVpluginDescriptionString:
            *((char **)aValue) = PLUGIN_DESCRIPTION; break;
        default:
            err = NPP_GetValue(future, aVariable, aValue);
    }
    return err;
}

char* NP_GetMIMEDescription(void)
{
    return NPP_GetMIMEDescription();
}

NPError NP_Initialize(NPNetscapeFuncs* browserFuncs, NPPluginFuncs* pluginFuncs)
{
    browser = browserFuncs;

    pluginFuncs->version = 11;
    pluginFuncs->size = sizeof(pluginFuncs);
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->setwindow = NPP_SetWindow;
    pluginFuncs->newstream = NPP_NewStream;
    pluginFuncs->destroystream = NPP_DestroyStream;
    pluginFuncs->asfile = NPP_StreamAsFile;
    pluginFuncs->writeready = NPP_WriteReady;
    pluginFuncs->write = NPP_Write;
    pluginFuncs->print = NPP_Print;
    pluginFuncs->event = NPP_HandleEvent;
    pluginFuncs->urlnotify = NPP_URLNotify;
    pluginFuncs->getvalue = NPP_GetValue;
    pluginFuncs->setvalue = NPP_SetValue;

    return NPERR_NO_ERROR;
}

NPError NP_Shutdown(void)
{
    return NPERR_NO_ERROR;
}

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
    if (browser->version >= 14)
        instance->pdata = browser->createobject (instance, getPluginClass());

    return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
    PluginObject *obj = instance->pdata;

    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
    PluginObject *obj = instance->pdata;
    
    // Do nothing if browser didn't support NPN_CreateObject which would have created the PluginObject.
    if (obj != NULL) {
        obj->window = window;
    }
    return NPERR_NO_ERROR;
}
 

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
    return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
    return NPERR_NO_ERROR;
}

int32 NPP_WriteReady(NPP instance, NPStream* stream)
{
    return 0;
}

int32 NPP_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
    return 0;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{

}

void NPP_Print(NPP instance, NPPrint* platformPrint)
{

}

int16 NPP_HandleEvent(NPP instance, void* event)
{
    return 0;
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{

}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
    if (variable == NPPVpluginScriptableNPObject) {
        void **v = (void **)value;
        PluginObject *obj = instance->pdata;
		// Increment reference count
		obj->referenceCount++;
        *v = obj;
        return NPERR_NO_ERROR;
    }
    return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
    return NPERR_GENERIC_ERROR;
}
