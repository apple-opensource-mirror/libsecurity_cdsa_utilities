/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */


//
// cssmaclpod - enhanced PodWrappers for ACL-related CSSM data structures
//
#ifdef __MWERKS__
#define _CPP_CSSMACLPOD
#endif

#include <security_cdsa_utilities/cssmaclpod.h>
#include <security_cdsa_utilities/cssmwalkers.h>


AuthorizationGroup::AuthorizationGroup(const AclAuthorizationSet &auths,
	Allocator &alloc)
{
	NumberOfAuthTags = auths.size();
	AuthTags = alloc.alloc<CSSM_ACL_AUTHORIZATION_TAG>(NumberOfAuthTags);
	copy(auths.begin(), auths.end(), AuthTags);	// happens to be sorted
}

void AuthorizationGroup::destroy(Allocator &alloc)
{
	alloc.free(AuthTags);
}

bool AuthorizationGroup::contains(CSSM_ACL_AUTHORIZATION_TAG tag) const
{
	return find(AuthTags, &AuthTags[NumberOfAuthTags], tag) != &AuthTags[NumberOfAuthTags];
}


AuthorizationGroup::operator AclAuthorizationSet() const
{
	return AclAuthorizationSet(AuthTags, &AuthTags[NumberOfAuthTags]);
}

AclEntryPrototype::AclEntryPrototype(const AclOwnerPrototype &proto)
{
	memset(this, 0, sizeof(*this));
	TypedSubject = proto.subject(); Delegate = proto.delegate();
	//@@@ set authorization to "is owner" pseudo-auth? See cssmacl.h
}

void AclEntryPrototype::tag(const char *tagString)
{
	if (tagString == NULL)
		EntryTag[0] = '\0';
	else if (strlen(tagString) > CSSM_MODULE_STRING_SIZE)
		CssmError::throwMe(CSSM_ERRCODE_INVALID_ACL_ENTRY_TAG);
	strcpy(EntryTag, tagString);
}


AclOwnerPrototype *AutoAclOwnerPrototype::make()
{
	if (!mAclOwnerPrototype) {
		mAclOwnerPrototype = new AclOwnerPrototype; 
		mAclOwnerPrototype->clearPod();
	}
	return mAclOwnerPrototype;
}

AutoAclOwnerPrototype::~AutoAclOwnerPrototype()
{
	if (mAllocator)
		DataWalkers::chunkFree(mAclOwnerPrototype, *mAllocator);
}

void
AutoAclOwnerPrototype::allocator(Allocator &allocator)
{
	mAllocator = &allocator;
}


AutoAclEntryInfoList::~AutoAclEntryInfoList()
{
	if (mAllocator)
	{
		DataWalkers::ChunkFreeWalker w(*mAllocator);
		for (uint32 ix = 0; ix < mNumberOfAclEntries; ix++)
			walk(w, mAclEntryInfo[ix]);
		//DataWalkers::chunkFree(mAclEntryInfo[ix], *mAllocator);	
		mAllocator->free(mAclEntryInfo);
	}
}

void
AutoAclEntryInfoList::allocator(Allocator &allocator)
{
	mAllocator = &allocator;
}