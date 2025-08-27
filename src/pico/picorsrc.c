#include "picorsrc.h"



/**
 * Returns non-zero if 'this' is a valid resource handle, zero otherwise.
 */
picoos_int16 picoctrl_isValidResourceHandle(picorsrc_Resource this)
{
    return (this != NULL) && CHECK_MAGIC_NUMBER(this);
}


picorsrc_ResourceManager picorsrc_newResourceManager(picoos_MemoryManager mm, picoos_Common common /* , picoos_char * configFile */)
{
    picorsrc_ResourceManager this = picoos_allocate(mm,sizeof(*this));
    if (NULL != this) {
        /* initialize */
        this->common = common;
        this->numResources = 0;
        this->resources = NULL;
        this->freeResources = NULL;
        this->numVoices = 0;
        this->voices = NULL;
        this->freeVoices = NULL;
        this->numVdefs = 0;
        this->vdefs = NULL;
        this->freeVdefs = NULL;
    }
    return this;
}

void picorsrc_disposeResourceManager(picoos_MemoryManager mm, picorsrc_ResourceManager * this)
{
    if (NULL != (*this)) {
        /* terminate */
        picoos_deallocate(mm,(void *)this);
    }
}

/* load resource file. the type of resource file etc. are in the header,
 * then follows the directory, then the knowledge bases themselves (as byte streams) */

pico_status_t picorsrc_loadResource(picorsrc_ResourceManager this,
        picoos_char * fileName, picorsrc_Resource * resource)
{
    picorsrc_Resource res;
    picoos_uint32 headerlen, len,maxlen;
    picoos_file_header_t header;
    picoos_uint8 rem;
    pico_status_t status = PICO_OK;

    if (resource == NULL) {
        return PICO_ERR_NULLPTR_ACCESS;
    } else {
        *resource = NULL;
    }

    res = picorsrc_newResource(this->common->mm);

    if (NULL == res) {
        return picoos_emRaiseException(this->common->em,PICO_EXC_OUT_OF_MEM,NULL,NULL);
    }

    if (PICO_MAX_NUM_RESOURCES <= this->numResources) {
        picoos_deallocate(this->common->mm, (void *) &res);
        return picoos_emRaiseException(this->common->em,PICO_EXC_MAX_NUM_EXCEED,NULL,(picoos_char *)"no more than %i resources",PICO_MAX_NUM_RESOURCES);
    }

    /* ***************** parse file name for file type and parameters */

    if (PICO_OK != parse_resource_name(fileName)) {
        picoos_deallocate(this->common->mm, (void *) &res);
        return PICO_EXC_UNEXPECTED_FILE_TYPE;
    }

    /* ***************** get header info */

    /* open binary file for reading (no key, nrOfBufs, bufSize) */
    PICODBG_DEBUG(("trying to open file %s",fileName));
    if (!picoos_OpenBinary(this->common, &res->file, fileName)) {
        /* open didn't succeed */
        status = PICO_EXC_CANT_OPEN_FILE;
        PICODBG_ERROR(("can't open file %s",fileName));
        picoos_emRaiseException(this->common->em, PICO_EXC_CANT_OPEN_FILE,
                NULL, (picoos_char *) "%s", fileName);
    }
    if (PICO_OK == status) {
        status = readHeader(this, &header, &headerlen, res->file);
        /* res->file now positioned at first pos after header */
    }

    /* ***************** check header values */
    if (PICO_OK == status && isResourceLoaded(this, header.field[PICOOS_HEADER_NAME].value)) {
        /* lingware is allready loaded, do nothing */
        PICODBG_WARN((">>> lingware '%s' allready loaded",header.field[PICOOS_HEADER_NAME].value));
        picoos_emRaiseWarning(this->common->em,PICO_WARN_RESOURCE_DOUBLE_LOAD,NULL,(picoos_char *)"%s",header.field[PICOOS_HEADER_NAME].value);
        status = PICO_WARN_RESOURCE_DOUBLE_LOAD;
    }

    if (PICO_OK == status) {
            /* get data length */
        status = picoos_read_pi_uint32(res->file, &len);
        PICODBG_DEBUG(("found net resource len of %i",len));
        /* allocate memory */
        if (PICO_OK == status) {
            PICODBG_TRACE((">>> 2"));
            maxlen = len + PICOOS_ALIGN_SIZE; /* once would be sufficient? */
            res->raw_mem = picoos_allocProtMem(this->common->mm, maxlen);
            /* res->size = maxlen; */
            status = (NULL == res->raw_mem) ? PICO_EXC_OUT_OF_MEM : PICO_OK;
        }
        if (PICO_OK == status) {
            rem = (uintptr_t) res->raw_mem % PICOOS_ALIGN_SIZE;
            if (rem > 0) {
                res->start = res->raw_mem + (PICOOS_ALIGN_SIZE - rem);
            } else {
                res->start = res->raw_mem;
            }

            /* read file contents into memory */
            status = (picoos_ReadBytes(res->file, res->start, &len)) ? PICO_OK
                    : PICO_ERR_OTHER;
            /* resources are read-only; the following write protection
             has an effect in test configurations only */
            picoos_protectMem(this->common->mm, res->start, len, /*enable*/TRUE);
        }
        /* note resource unique name */
        if (PICO_OK == status) {
            if (picoos_strlcpy(res->name,header.field[PICOOS_HEADER_NAME].value,PICORSRC_MAX_RSRC_NAME_SIZ) < PICORSRC_MAX_RSRC_NAME_SIZ) {
                PICODBG_DEBUG(("assigned name %s to resource",res->name));
                status = PICO_OK;
            } else {
                status = PICO_ERR_INDEX_OUT_OF_RANGE;
                PICODBG_ERROR(("failed assigning name %s to resource",
                               res->name));
                picoos_emRaiseException(this->common->em,
                                        PICO_ERR_INDEX_OUT_OF_RANGE, NULL,
                                        (picoos_char *)"resource %s",res->name);
            }
        }

        /* get resource type */
        if (PICO_OK == status) {
            if (!picoos_strcmp(header.field[PICOOS_HEADER_CONTENT_TYPE].value, PICORSRC_FIELD_VALUE_TEXTANA)) {
                res->type = PICORSRC_TYPE_TEXTANA;
            } else if (!picoos_strcmp(header.field[PICOOS_HEADER_CONTENT_TYPE].value, PICORSRC_FIELD_VALUE_SIGGEN)) {
                res->type = PICORSRC_TYPE_SIGGEN;
            } else if (!picoos_strcmp(header.field[PICOOS_HEADER_CONTENT_TYPE].value, PICORSRC_FIELD_VALUE_SIGGEN)) {
                res->type = PICORSRC_TYPE_USER_LEX;
            } else if (!picoos_strcmp(header.field[PICOOS_HEADER_CONTENT_TYPE].value, PICORSRC_FIELD_VALUE_SIGGEN)) {
                res->type = PICORSRC_TYPE_USER_PREPROC;
            } else {
                res->type = PICORSRC_TYPE_OTHER;
            }
        }

        if (PICO_OK == status) {
            /* create kb list from resource */
            status = picorsrc_getKbList(this, res->start, len, &res->kbList);
        }
    }

    if (status == PICO_OK) {
        /* add resource to rm */
        res->next = this->resources;
        this->resources = res;
        this->numResources++;
        *resource = res;
        PICODBG_DEBUG(("done loading resource %s from %s", res->name, fileName));
    } else {
        picorsrc_disposeResource(this->common->mm, &res);
        PICODBG_ERROR(("failed to load resource"));
    }

    if (status < 0) {
        return status;
    } else {
        return PICO_OK;
    }
}

/* unload resource file. (if resource file is busy, warn and don't unload) */
pico_status_t picorsrc_unloadResource(picorsrc_ResourceManager this, picorsrc_Resource * resource) {

    picorsrc_Resource r1, r2, rsrc;

    if (resource == NULL) {
        return PICO_ERR_NULLPTR_ACCESS;
    } else {
        rsrc = *resource;
    }

    if (rsrc->lockCount > 0) {
        return PICO_EXC_RESOURCE_BUSY;
    }
    /* terminate */
    if (rsrc->file != NULL) {
        picoos_CloseBinary(this->common, &rsrc->file);
    }
    if (NULL != rsrc->raw_mem) {
        picoos_deallocProtMem(this->common->mm, (void *) &rsrc->raw_mem);
        PICODBG_DEBUG(("deallocated raw mem"));
    }

    r1 = NULL;
    r2 = this->resources;
    while (r2 != NULL && r2 != rsrc) {
        r1 = r2;
        r2 = r2->next;
    }
    if (NULL == r1) {
        this->resources = rsrc->next;
    } else if (NULL == r2) {
        /* didn't find resource in rm! */
        return PICO_ERR_OTHER;
    } else {
        r1->next = rsrc->next;
    }

    if (NULL != rsrc->kbList) {
        picorsrc_releaseKbList(this, &rsrc->kbList);
    }

    picoos_deallocate(this->common->mm,(void **)resource);
    this->numResources--;

    return PICO_OK;
}


pico_status_t picorsrc_createDefaultResource(picorsrc_ResourceManager this
        /*, picorsrc_Resource * resource */)
{
    picorsrc_Resource res;
    pico_status_t status = PICO_OK;


    /* *resource = NULL; */

    if (PICO_MAX_NUM_RESOURCES <= this->numResources) {
        return picoos_emRaiseException(this->common->em,PICO_EXC_MAX_NUM_EXCEED,NULL,(picoos_char *)"no more than %i resources",PICO_MAX_NUM_RESOURCES);
    }

    res = picorsrc_newResource(this->common->mm);

    if (NULL == res) {
        return picoos_emRaiseException(this->common->em,PICO_EXC_OUT_OF_MEM,NULL,NULL);
    }

    if (picoos_strlcpy(res->name,PICOKNOW_DEFAULT_RESOURCE_NAME,PICORSRC_MAX_RSRC_NAME_SIZ) < PICORSRC_MAX_RSRC_NAME_SIZ) {
        PICODBG_DEBUG(("assigned name %s to default resource",res->name));
        status = PICO_OK;
    } else {
        PICODBG_ERROR(("failed assigning name %s to default resource",res->name));
        status = PICO_ERR_INDEX_OUT_OF_RANGE;
    }
    status = picorsrc_createKnowledgeBase(this, NULL, 0, (picoknow_kb_id_t)PICOKNOW_KBID_FIXED_IDS, &res->kbList);

    if (PICO_OK == status) {
        res->next = this->resources;
        this->resources = res;
        this->numResources++;
        /* *resource = res; */

    }


    return status;

}

pico_status_t picorsrc_rsrcGetName(picorsrc_Resource this,
        picoos_char * name, picoos_uint32 maxlen) {
    if (!picoctrl_isValidResourceHandle(this)) {
        return PICO_ERR_INVALID_ARGUMENT;
    }
    picoos_strlcpy(name, this->name,maxlen);
    return PICO_OK;
}


pico_status_t picorsrc_addResourceToVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char * voiceName, picoos_char * resourceName)
{
    picorsrc_VoiceDefinition vdef;

    if (NULL == this) {
        PICODBG_ERROR(("this is NULL"));
        return PICO_ERR_NULLPTR_ACCESS;
    }
    if ((PICO_OK == findVoiceDefinition(this,voiceName,&vdef)) && (NULL != vdef)) {
        if (PICO_MAX_NUM_RSRC_PER_VOICE <= vdef->numResources) {
            return picoos_emRaiseException(this->common->em,PICO_EXC_MAX_NUM_EXCEED,NULL,(picoos_char *)"no more than %i resources per voice",PICO_MAX_NUM_RSRC_PER_VOICE);
        }
        if (picoos_strlcpy(vdef->resourceName[vdef->numResources++], resourceName,
                            PICORSRC_MAX_RSRC_NAME_SIZ) < PICORSRC_MAX_RSRC_NAME_SIZ) {
            PICODBG_DEBUG(("vdef added resource '%s' to voice '%s'",resourceName,voiceName));
            return PICO_OK;
        } else {
            PICODBG_ERROR(("illegal name (%s)",resourceName));
            return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_ILLEGAL,NULL,(picoos_char *)"%s",resourceName);
        }

    } else {
        return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_UNDEFINED,NULL,(picoos_char *)"%s",voiceName);
    }
}


pico_status_t picorsrc_createVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char * voiceName)
{
    picorsrc_VoiceDefinition vdef;

    if (NULL == this) {
        PICODBG_ERROR(("this is NULL"));
        return PICO_ERR_NULLPTR_ACCESS;
    }
    if ((PICO_OK == findVoiceDefinition(this,voiceName,&vdef)) && (NULL != vdef)) {
        PICODBG_ERROR(("voice %s allready defined",voiceName));
        return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_CONFLICT,NULL,NULL);
    }
    if (PICO_MAX_NUM_VOICE_DEFINITIONS <= this->numVdefs) {
        PICODBG_ERROR(("max number of vdefs exceeded (%i)",this->numVdefs));
        return picoos_emRaiseException(this->common->em,PICO_EXC_MAX_NUM_EXCEED,NULL,(picoos_char *)"no more than %i voice definitions",PICO_MAX_NUM_VOICE_DEFINITIONS);
    }
    if (NULL == this->freeVdefs) {
        vdef = picorsrc_newVoiceDefinition(this->common->mm);
    } else {
        vdef = this->freeVdefs;
        this->freeVdefs = vdef->next;
        vdef->voiceName[0] = NULLC;
        vdef->numResources = 0;
        vdef->next = NULL;
    }
    if (NULL == vdef) {
        return picoos_emRaiseException(this->common->em,PICO_EXC_OUT_OF_MEM,NULL,NULL);
    }
    if (picoos_strlcpy(vdef->voiceName, voiceName,
            PICO_MAX_VOICE_NAME_SIZE) < PICO_MAX_VOICE_NAME_SIZE) {
        vdef->next = this->vdefs;
        this->vdefs = vdef;
        this->numVdefs++;
        if (PICO_OK != picorsrc_addResourceToVoiceDefinition(this,voiceName,PICOKNOW_DEFAULT_RESOURCE_NAME)) {
            return picoos_emRaiseException(this->common->em,PICO_ERR_OTHER,NULL,(picoos_char *)"problem loading default resource %s",voiceName);
        }
        PICODBG_DEBUG(("vdef created (%s)",voiceName));
        return PICO_OK;
    } else {
        PICODBG_ERROR(("illegal name (%s)",voiceName));
        return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_ILLEGAL,NULL,(picoos_char *)"%s",voiceName);
    }
}


pico_status_t picorsrc_releaseVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char *voiceName)
{
    picorsrc_VoiceDefinition v, l;

    if (this == NULL) {
        return PICO_ERR_NULLPTR_ACCESS;
    }

    l = NULL;
    v = this->vdefs;
    while ((v != NULL) && (picoos_strcmp(v->voiceName, voiceName) != 0)) {
        l = v;
        v = v->next;
    }
    if (v != NULL) {
        /* remove v from vdefs list */
        if (l != NULL) {
            l->next = v->next;
        } else {
            this->vdefs = v->next;
        }
        /* insert v at head of freeVdefs list */
        v->next = this->freeVdefs;
        this->freeVdefs = v;
        this->numVdefs--;
        return PICO_OK;
    } else {
        /* we should rather return a warning, here */
        /* return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_UNDEFINED,"%s", NULL); */
        return PICO_OK;
    }
}



/* ******* accessing voices **************************************/


/* create voice, given a voice name. the corresponding lock counts are incremented */

pico_status_t picorsrc_createVoice(picorsrc_ResourceManager this, const picoos_char * voiceName, picorsrc_Voice * voice) {

    picorsrc_VoiceDefinition vdef;
    picorsrc_Resource rsrc;
    picoos_uint8 i, required;
    picoknow_KnowledgeBase kb;
    /* pico_status_t status = PICO_OK; */

    PICODBG_DEBUG(("creating voice %s",voiceName));

    if (NULL == this) {
        PICODBG_ERROR(("this is NULL"));
        return PICO_ERR_NULLPTR_ACCESS;

    }
    /* check number of voices */
    if (PICORSRC_MAX_NUM_VOICES <= this->numVoices) {
        PICODBG_ERROR(("PICORSRC_MAX_NUM_VOICES exceeded"));
        return picoos_emRaiseException(this->common->em,PICO_EXC_MAX_NUM_EXCEED,NULL,(picoos_char *)"no more than %i voices",PICORSRC_MAX_NUM_VOICES);
    }

    /* find voice definition for that name */
    if (!(PICO_OK == findVoiceDefinition(this,voiceName,&vdef)) || (NULL == vdef)) {
        PICODBG_ERROR(("no voice definition for %s",voiceName));
        return picoos_emRaiseException(this->common->em,PICO_EXC_NAME_UNDEFINED,NULL,(picoos_char *)"voice definition %s",voiceName);

    }
    PICODBG_DEBUG(("found voice definition for %s",voiceName));

    /* check that resources are loaded */
    for (i = 0; i < vdef->numResources; i++) {
        required = (NULLC != vdef->resourceName[i][0]);
        if (required && !isResourceLoaded(this,vdef->resourceName[i])) {
            PICODBG_ERROR(("resource missing"));
            return picoos_emRaiseException(this->common->em,PICO_EXC_RESOURCE_MISSING,NULL,(picoos_char *)"resource %s for voice %s",vdef->resourceName[i],voiceName);
        }
    }

    /* allocate new voice */
    if (NULL == this->freeVoices) {
        *voice = picorsrc_newVoice(this->common->mm);
    } else {
        *voice = this->freeVoices;
        this->freeVoices = (*voice)->next;
        picorsrc_initializeVoice(*voice);
    }
    if (*voice == NULL) {
        return picoos_emRaiseException(this->common->em, PICO_EXC_OUT_OF_MEM, NULL, NULL);
    }
    this->numVoices++;

    /* copy resource kb pointers into kb array of voice */
    for (i = 0; i < vdef->numResources; i++) {
        required = (NULLC != vdef->resourceName[i][0]);
        if (required) {
            findResource(this,vdef->resourceName[i],&rsrc);
           (*voice)->resourceArray[(*voice)->numResources++] = rsrc;
            rsrc->lockCount++;
            kb = rsrc->kbList;
            while (NULL != kb) {
                if (NULL != (*voice)->kbArray[kb->id]) {
                    picoos_emRaiseWarning(this->common->em,PICO_WARN_KB_OVERWRITE,NULL, (picoos_char *)"%i", kb->id);
                    PICODBG_WARN(("overwriting knowledge base of id %i", kb->id));

                }
                PICODBG_DEBUG(("setting knowledge base of id %i", kb->id));

                (*voice)->kbArray[kb->id] = kb;
                kb = kb->next;
            }
        }
    } /* for */

    return PICO_OK;
}

/* dispose voice. the corresponding lock counts are decremented. */

pico_status_t picorsrc_releaseVoice(picorsrc_ResourceManager this, picorsrc_Voice * voice)
{
    picoos_uint16 i;
    picorsrc_Voice v = *voice;
    if (NULL == this || NULL == v) {
        return PICO_ERR_NULLPTR_ACCESS;
    }
    for (i = 0; i < v->numResources; i++) {
        v->resourceArray[i]->lockCount--;
    }
    v->next = this->freeVoices;
    this->freeVoices = v;
    this->numVoices--;

    return PICO_OK;
}