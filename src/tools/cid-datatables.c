/***********************************************************************
*
* Program:
*   Conky Images Display
*
* License :
*  This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License, version 2.
*   If you don't know what that means take a look at:
*      http://www.gnu.org/licenses/licenses.html#GPL
*
* Original idea :
*   Charlie MERLAND, July 2008.
*
***********************************************************************/
/*
   *
   *                  cid-datatables.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-datatables.h"

CidDataTable *
cid_datatable_new (void)
{
    CidDataTable *res = g_new0(CidDataTable,1);
    if (res != NULL)
    {
        res->length = 0;
        res->head = NULL;
        res->tail = NULL;
    }
    return res;
}

CidDataCase *
cid_datacase_new (void)
{
    CidDataCase *ret = g_new0(CidDataCase,1);
    if (ret != NULL) 
    {
        ret->content = NULL;
        ret->next = NULL;
        ret->prev = NULL;
    }
    return ret;
}

CidDataContent *
cid_datacontent_new (GType iType, gpointer value)
{
    CidDataContent *ret = g_new0(CidDataContent,1);
    if (ret != NULL)
    {
        ret->type = iType;
        switch (iType) 
        {
            case G_TYPE_STRING:
                ret->string = NULL;
                int iLength = (strlen((gchar *) value)+1)*sizeof(gchar);
                ret->string = g_malloc0(iLength);
                strncpy(ret->string, (gchar *) value, iLength);
                break;
            case G_TYPE_INT:
                ret->iNumber = (gint)(long) value;
                break;
            case G_TYPE_BOOLEAN:
                ret->booleen = (gboolean)(long) value;
                break;
            case CID_TYPE_SUBSTITUTE:
                ret->sub = (CidSubstitute *) value;
                break;
            default:
                g_free(ret);
                return NULL;
        }
    }
    return ret;
}

gboolean
cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2)
{
    if (d1 == NULL || d2 == NULL)
        return FALSE;
    if (d1->type != d2->type)
        return FALSE;
    switch (d1->type) 
    {
        case G_TYPE_STRING:
            return g_strcmp0(d1->string,d2->string) == 0;
        case G_TYPE_INT:
            return d1->iNumber == d2->iNumber;
        case G_TYPE_BOOLEAN:
            return d1->booleen == d2->booleen;
        case CID_TYPE_SUBSTITUTE:
            return g_strcmp0(d1->sub->regex,d2->sub->regex) == 0
                   && g_strcmp0(d1->sub->replacement,
                                d2->sub->replacement) == 0;
    }
}

void
cid_datatable_append(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL) 
    {
        CidDataCase *p_new = cid_datacase_new(); 
        if (p_new != NULL) 
        {
            p_new->content = data; 
            p_new->next = NULL; 
            if ((*p_list)->tail == NULL)
            {
                p_new->prev = NULL; 
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->tail->next = p_new;
                p_new->prev = (*p_list)->tail;
                (*p_list)->tail = p_new;
            }
            (*p_list)->length++;
        }
    }
}

void
cid_datatable_prepend(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_new = cid_datacase_new();
        if (p_new != NULL)
        {
            p_new->content = data;
            p_new->prev = NULL;
            if ((*p_list)->tail == NULL)
            {
                p_new->next = NULL;
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->head->prev = p_new;
                p_new->next = (*p_list)->head;
                (*p_list)->head = p_new;
            }
            (*p_list)->length++;
       }
    }
}

void
cid_datatable_insert(CidDataTable **p_list, 
                     CidDataContent *data, 
                     gint position)
{
    if (*p_list != NULL)
    {
        if (position < 0)
        {
            cid_datatable_prepend(p_list,data);
            return;
        }
        else 
        if (position > (gint) cid_datatable_length(*p_list))
        {
            cid_datatable_append(p_list,data);
            return;
        }
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    cid_datatable_append(p_list, data);
                }
                else if (p_temp->prev == NULL)
                {
                    cid_datatable_prepend(p_list, data);
                }
                else
                {
                    CidDataCase *p_new = cid_datacase_new();
                    if (p_new != NULL)
                    {
                        p_new->content = data;
                        p_temp->next->prev = p_new;
                        p_temp->prev->next = p_new;
                        p_new->prev = p_temp->prev;
                        p_new->next = p_temp;
                        (*p_list)->length++;
                    }
                }
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
}

CidDataCase *
cid_datatable_get_id (CidDataTable *pTable, gint iPos)
{
    g_return_val_if_fail (pTable != NULL, NULL);
    
    size_t iSize = cid_datatable_length (pTable);
    
    g_return_val_if_fail (iPos < (gint)iSize && iPos > -1, NULL);
    
    CidDataCase *pRet = NULL;
    gint cpt = 0;
    if (iPos < (gint)iSize/2)
    {
        CidDataCase *pCase = pTable->head;
        pRet = pCase;
        for (; cpt < iPos; cpt++)
        {
            pRet = pCase->next;
            pCase = pRet;
        }
    }
    else
    {
        CidDataCase *pCase = pTable->tail;
        pRet = pCase;
        cpt = iSize - 1;
        for (; cpt > iPos; cpt --)
        {
            pRet = pCase->prev;
            pCase = pRet;
        }
    }
    return pRet;
}

void
cid_free_datacase_full (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL)
    {
        cid_free_datacontent(pCase->content);
        g_free(pCase);
    }
}

void
cid_free_datacontent_full (CidDataContent *pContent, gpointer *pData)
{
    if (pContent != NULL)
    {
        if (pContent->type == G_TYPE_STRING && pContent->string != NULL)
            g_free(pContent->string);
        if (pContent->type == CID_TYPE_SUBSTITUTE)
            cid_free_substitute (pContent->sub);
        g_free(pContent);
    }
}

void
cid_free_datatable (CidDataTable *p_list)
{
    if (p_list != NULL)
    {
        cid_datatable_foreach (p_list,
                               (CidDataAction) cid_free_datacase_full, 
                               NULL);
    }
}

void
cid_clear_datatable (CidDataTable **p_list)
{
    if (*p_list != NULL)
    {
        cid_free_datatable (*p_list);
        g_free(*p_list), *p_list = NULL;
    }
}

static CidDataCase *
cid_clone_datacase (const CidDataCase *pCase)
{
    CidDataCase *res = cid_datacase_new ();
    if (res != NULL)
    {
        CidDataContent *ori = pCase->content;
        CidDataContent *new;
        switch (ori->type)
        {
            case G_TYPE_INT: 
                new = 
                    cid_datacontent_new_int (GINT_TO_POINTER(
                                    pCase->content->iNumber));
                break;
            case G_TYPE_BOOLEAN:
                new = 
                    cid_datacontent_new_boolean (GINT_TO_POINTER(
                                    ori->booleen));
                break;
            case G_TYPE_STRING:
                new = cid_datacontent_new_string (ori->string);
                break;
            case CID_TYPE_SUBSTITUTE:
                new = 
                    cid_datacontent_new_substitute (
                            cid_new_substitute (ori->sub->regex,
                                                ori->sub->replacement));
                break;
            default:
                return NULL;
        }
        res->content = new;
        res->next = pCase->next;
        res->prev = pCase->prev;
    }
    return res;
}

void
cid_datatable_foreach (CidDataTable *p_list, 
                       CidDataAction func, 
                       gpointer *pData)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        gboolean bCreateData = FALSE;
        if (pData == NULL)
        {
            pData = g_new(gpointer,1);
            bCreateData = TRUE;
        }
        gint cpt = 1;
        while (p_temp != NULL)
        {
            pData[0] = GINT_TO_POINTER(cpt);
            CidDataCase *p_del = cid_clone_datacase (p_temp);
            func (p_temp, pData);
            p_temp = p_del->next;
            cid_free_datacase (p_del);
            cpt++;
        }
        if (bCreateData)
        {
            g_free(pData);
        }
    }
}

void
cid_datacase_print (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL && pCase->content != NULL)
    {
        switch (pCase->content->type) 
        {
            case G_TYPE_STRING:
                fprintf (stdout,"%s\n",pCase->content->string);
                break;
            case G_TYPE_INT:
                fprintf (stdout,"%d\n",pCase->content->iNumber);
                break;
            case G_TYPE_BOOLEAN:
                fprintf (stdout,
                         "%s\n",
                         pCase->content->booleen ? "TRUE" : "FALSE");
                break;
            case CID_TYPE_SUBSTITUTE:
                fprintf (stdout,
                         "%s>%s\n",
                         pCase->content->sub->regex,
                         pCase->content->sub->replacement);
                break;
        }
    }
}

void
cid_datatable_remove(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        gboolean found = FALSE;
        while (p_temp != NULL && !found)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                if (p_temp->next == NULL)
                {
                    cid_free_datacase((*p_list)->tail);
                    (*p_list)->tail = p_temp->prev;
                    cid_free_datacase((*p_list)->tail->next);
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    cid_free_datacase((*p_list)->head);
                    (*p_list)->head = p_temp->next;
                    cid_free_datacase((*p_list)->tail->prev);
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
                found = TRUE;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_all(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        while (p_temp != NULL)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                CidDataCase *p_del = p_temp;
                p_temp = p_temp->next;
                if (p_del->next == NULL)
                {
                    (*p_list)->tail = p_del->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_del->prev == NULL)
                {
                    (*p_list)->head = p_del->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_del->next->prev = p_del->prev;
                    p_del->prev->next = p_del->next;
                }
                cid_free_datacase(p_del);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_id(CidDataTable **p_list, gint position)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    (*p_list)->tail = p_temp->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    (*p_list)->head = p_temp->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
}

size_t
cid_datatable_length(CidDataTable *p_list)
{
    size_t ret = 0;
    if (p_list != NULL)
    {
        ret = p_list->length;
    }
    return ret;
}

CidDataTable *
cid_create_datatable (GType iDataType, ...)
{
    CidDataTable *res = cid_datatable_new();
    GType iCurrType = iDataType;
    va_list args;
    va_start(args,iDataType);
    void *current;
    while ((GType)(current = va_arg(args,gpointer)) != G_TYPE_INVALID) {
        CidDataContent *tmp = NULL;
        if ((GType) current == G_TYPE_BOOLEAN || 
            (GType) current == G_TYPE_INT ||
            (GType) current == G_TYPE_STRING || 
            (GType) current == CID_TYPE_SUBSTITUTE)
        {
            iCurrType = (GType) current;
            continue;
        }
        switch (iCurrType) 
        {
            case G_TYPE_BOOLEAN:
                tmp = cid_datacontent_new_boolean(current);
                break;
            case G_TYPE_STRING:
                tmp = cid_datacontent_new_string(current);
                break;
            case G_TYPE_INT:
                tmp = cid_datacontent_new_int(current);
                break;
            case CID_TYPE_SUBSTITUTE:
                tmp = cid_datacontent_new_substitute(current);
                break;
            default:
                iCurrType = (GType) current;
        }
        cid_datatable_append(&res,tmp);
    }
    va_end(args);
    return res;
}

CidDataTable *
cid_create_sized_datatable_with_default_full (size_t iSize, 
                                              GType iType, 
                                              void *value)
{
    size_t cpt = 0;
    CidDataTable *res = cid_datatable_new();
    for (;cpt<iSize;cpt++)
    {
        CidDataContent *tmp = cid_datacontent_new(iType, value);
        cid_datatable_append(&res,tmp);
    }
    return res;
}

CidDataTable *
cid_char_table_to_datatable (gchar **table, gint iSize)
{
    g_return_val_if_fail (table != NULL,NULL);

    gint cpt = 0;
    CidDataTable *res = cid_datatable_new();
    if (res != NULL)
    {
        while (iSize == -1 ? 
                    table[cpt] != NULL : 
                    (cpt<iSize && table[cpt] != NULL))
        {
            CidDataContent *tmp = cid_datacontent_new (G_TYPE_STRING, 
                                                       table[cpt]);
            cid_datatable_append(&res,tmp);
            cpt++;
        } 
    }
    return res;
}

gchar **
cid_datatable_to_char_table (CidDataTable *pTable, gint *iSize)
{
    g_return_val_if_fail (pTable != NULL, NULL);
    
    size_t size = cid_datatable_length (pTable);
    gchar **res = g_malloc0_n (size,sizeof(gchar *));
    gint cpt = 0;
    if (res != NULL) 
    {
        BEGIN_FOREACH_DT (pTable)
            g_free (res[cpt]);
            res[cpt] = g_strdup (p_temp->content->string);
            cpt++;
        END_FOREACH_DT_NF
    
        *iSize = cpt;
    } 
    else 
    {
        *iSize = -1;
    }
    return res;
}

static void
cid_copy_datacase (CidDataCase *pCase, gpointer *pData)
{
    CidDataTable *table = (CidDataTable *)pData[1];
    CidDataContent *ori = pCase->content;
    CidDataContent *new;
    switch (ori->type)
    {
        case G_TYPE_INT: 
            new = cid_datacontent_new_int (GINT_TO_POINTER(
                                            pCase->content->iNumber));
            break;
        case G_TYPE_BOOLEAN:
            new = cid_datacontent_new_boolean (GINT_TO_POINTER(
                                                ori->booleen));
            break;
        case G_TYPE_STRING:
            new = cid_datacontent_new_string (ori->string);
            break;
        case CID_TYPE_SUBSTITUTE:
            new = cid_datacontent_new_substitute (
                            cid_new_substitute (ori->sub->regex,
                                                ori->sub->replacement));
            break;
        default:
            new = NULL;
    }
    if (new != NULL)
    {
        cid_datatable_append (&table,new);
    }
}

CidDataTable *
cid_clone_datatable (CidDataTable *pSource)
{
    CidDataTable *res = cid_datatable_new ();
    if (res != NULL)
    {
        gpointer *data = g_new0 (gpointer,2);
        data[0] = GINT_TO_POINTER (0);
        data[1] = res;
        cid_datatable_foreach (pSource, cid_copy_datacase, data);
        g_free (data);
    }
    return res;
}
