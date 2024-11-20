/*
 * This file is part of OpenOrion2
 * Copyright (C) 2021 Martin Doucha
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef LBX_H_
#define LBX_H_

#include <stdexcept>
#include "stream.h"
#include "utils.h"

#define LANG_ENGLISH 0
#define LANG_GERMAN 1
#define LANG_FRENCH 2
#define LANG_SPANISH 3
#define LANG_ITALIAN 4
#define LANG_COUNT 5

#define TXT_MISC_BILLTEXT 0
#define TXT_MISC_BILLTEX2 1
#define TXT_MISC_JIMTEXT 2
#define TXT_MISC_JIMTEXT2 3
#define TXT_MISC_KENTEXT 4
#define TXT_MISC_KENTEXT1 5
#define TXT_MISC_COUNT 6
#define TXT_TECH_SPECIAL_NAME 0
#define TXT_TECH_SPECIAL_DESC 1
#define TXT_TECH_WEAPON_NAME 2
#define TXT_TECH_WEAPON_DESC 3
#define TXT_TECH_COUNT 4
#define TXT_HELPSECTION_COUNT 16

struct HelpText {
	char *title, *text, *archive;
	unsigned asset_id, frame;	// Image to display in help window
	unsigned section;	// Help section (buildings/armor/weapons/...)
	unsigned nextParagraph;	// !=0: Text continues in another entry

	HelpText(void);
	HelpText(const HelpText &other);
	~HelpText(void);

	const HelpText &operator=(HelpText other);
};

struct HelpLink {
	char *title;
	unsigned id;	// Reference to HelpText entry

	HelpLink(void);
	HelpLink(const HelpLink &other);
	~HelpLink(void);

	const HelpLink &operator=(HelpLink other);
};

class LBXArchive {
private:
	struct LBXEntry {
		size_t offset, size;
	};

	File _file;
	unsigned _assetCount;
	struct LBXEntry *_index;

	// Do NOT implement
	LBXArchive(const LBXArchive &other);
	const LBXArchive &operator=(const LBXArchive &other);

public:
	explicit LBXArchive(const char *filename);
	~LBXArchive(void);

	const char *filename(void) const;
	unsigned assetCount(void) const;

	MemoryReadStream *loadAsset(unsigned id);
};

class TextManager : public Recyclable {
private:
	struct StringList {
	private:
		// Do NOT implement
		StringList(const StringList &other);
		const StringList &operator=(const StringList &other);

	protected:
		void clear(void);

	public:
		char **data;
		unsigned size;

		StringList(void);
		~StringList(void);

		const char *operator[](unsigned id) const;

		// Multiple assets, one string each
		void loadFile(const char *filename, unsigned offset,
			unsigned step, unsigned group_id, unsigned groups);

		// Single asset, multiple string blocks
		void loadAsset(const char *filename, unsigned asset_id);

		// Single asset, single string block with multiple strings
		void loadStrings(const char *filename, unsigned asset_id,
			unsigned offset);
	};

	// billtext, jimtext, kentext
	struct StringList _misctext[TXT_MISC_COUNT];
	struct StringList _antarmsg;
	struct StringList _councmsg;
	struct StringList _maintext;
	struct StringList _eventmsg;
	struct StringList _rstring;
	struct StringList _credits;
	struct StringList _skillname;
	struct StringList _skilldesc;
	struct StringList _techdesc[TXT_TECH_COUNT];
	struct StringList _racename;
	struct StringList _shipname;
	struct StringList _homeworlds;
	struct StringList _starname;
	struct StringList _estrings;
	struct StringList _hstrings;
	struct StringList _raceTraits;	// racestuf.lbx assets 0-5
	struct StringList _raceInfo;	// racestuf.lbx assets 8-13
	struct StringList _techname;

	char **_officerTitle;
	struct StringList *_diplomsg;
	unsigned _diplomsgCount;

	// help asset 0
	struct HelpText *_help;
	unsigned _helpCount;

	// help assets 1-16
	struct HelpLink *_helpIndex[TXT_HELPSECTION_COUNT];
	unsigned _helpIndexCount[TXT_HELPSECTION_COUNT];

	// Do NOT implement
	TextManager(const TextManager &other);
	const TextManager &operator=(const TextManager &other);

protected:
	void clear(void);

	void loadDiplomsg(unsigned lang_id);
	void loadHelp(unsigned lang_id);
	void loadOfficerTitles(unsigned lang_id);
	void load(unsigned lang_id);

public:
	TextManager(unsigned lang_id);
	~TextManager(void);

	const char *antarmsg(unsigned str_id) const;
	const char *councmsg(unsigned str_id) const;
	const char *misctext(unsigned file, unsigned str_id) const;
	const char *maintext(unsigned str_id) const;
	const char *eventmsg(unsigned str_id) const;
	const char *rstring(unsigned str_id) const;
	const char *credits(unsigned str_id) const;
	const char *skillname(unsigned str_id) const;
	const char *skilldesc(unsigned str_id) const;
	const char *techdesc(unsigned asset_id, unsigned str_id) const;
	const char *racename(unsigned str_id) const;
	const char *shipname(unsigned str_id) const;
	const char *homeworlds(unsigned str_id) const;
	const char *starname(unsigned str_id) const;
	const char *estrings(unsigned str_id) const;
	const char *hstrings(unsigned str_id) const;
	const char *raceTraits(unsigned str_id) const;
	const char *raceInfo(unsigned str_id) const;
	const char *techname(unsigned str_id) const;
	const char *officerTitle(unsigned officer_id) const;
	const char *diplomsg(unsigned asset_id, unsigned str_id) const;
	const struct HelpText *help(unsigned id) const;
	const struct HelpLink *helpIndex(unsigned section_id,
		unsigned entry_id) const;
};

class AssetManager;
class Image;

class LBXCacheEntry {
public:
	LBXCacheEntry(void);
	virtual ~LBXCacheEntry(void);

	virtual void take(void) = 0;
	virtual void release(void) = 0;
};

class LBXAsset {
private:
	LBXCacheEntry *_cacheRef;

public:
	LBXAsset(void);
	virtual ~LBXAsset(void);

	// Bump the asset reference counter to ensure it does not get deleted
	// by another part of code. You must call release() later.
	void take(void);

	// Decrease the reference counter and delete the asset when it's
	// no longer in use. Each take() must be paired with a matching
	// release().
	void release(void);

	friend class AssetManager;
};

template <class C> class AssetPointer {
private:
	C *_asset;

protected:
	AssetPointer(C *asset);

public:
	AssetPointer(void);
	AssetPointer(const AssetPointer &other);
	~AssetPointer(void);

	const AssetPointer &operator=(const AssetPointer &other);
	C *operator->(void);

	// Explicit-only to avoid accidental asset release immediately after
	// load. Image *img = (Image*)gameAssets->getImage(...) will not work
	// because the AssetPointer gets destroyed before you can call
	// img->take() and the just-loaded asset will be freed with it.
	explicit operator C*(void);
	explicit operator const C*(void) const;

	// Safely get raw pointer to the asset and bump the reference counter.
	C* take(void);

	friend class AssetManager;
};

typedef AssetPointer<Image> ImageAsset;

class AssetManager {
private:
	template <class C> class CacheEntry : public LBXCacheEntry {
	public:
		C *data;
		unsigned refs;

		CacheEntry(void);
		~CacheEntry(void);

		void take(void);
		void release(void);
	};

	struct FileCache {
		char *filename;
		size_t size;
		CacheEntry<Image> *images;
	};

	LBXArchive *_curfile;
	FileCache *_cache;
	size_t _cacheCount, _cacheSize;

protected:
	FileCache *getCache(const char *filename);
	void openArchive(FileCache *entry);
	FileCache *cacheImage(const char *filename, unsigned id,
		const uint8_t **palettes, unsigned palcount);

public:
	AssetManager(void);
	~AssetManager(void);

	// Get asset from cache, loading it from disk if necessary. Reference
	// counter gets automatically increased.
	ImageAsset getImage(const char *filename, unsigned id,
		const uint8_t *palette = NULL);
	ImageAsset getImage(const char *filename, unsigned id,
		const uint8_t **palettes, unsigned palcount);

	MemoryReadStream *rawData(const char *filename, unsigned id);
};

template <class C>
AssetPointer<C>::AssetPointer(void) : _asset(NULL) {

}

template <class C>
AssetPointer<C>::AssetPointer(C *asset) : _asset(asset) {
	if (_asset) {
		_asset->take();
	}
}

template <class C>
AssetPointer<C>::AssetPointer(const AssetPointer &other) :
	_asset(other._asset) {

	if (_asset) {
		_asset->take();
	}
}

template <class C>
AssetPointer<C>::~AssetPointer(void) {
	if (_asset) {
		_asset->release();
	}
}

template <class C>
const AssetPointer<C> &AssetPointer<C>::operator=(const AssetPointer &other) {
	if (_asset) {
		_asset->release();
	}

	_asset = other._asset;

	if (_asset) {
		_asset->take();
	}

	return *this;
}


template <class C>
C *AssetPointer<C>::operator->(void) {
	if (!_asset) {
		throw std::runtime_error("Member access on NULL asset pointer");
	}

	return _asset;
}

template <class C>
AssetPointer<C>::operator C*(void) {
	return _asset;
}

template <class C>
AssetPointer<C>::operator const C*(void) const {
	return _asset;
}

template <class C>
C* AssetPointer<C>::take(void) {
	if (_asset) {
		_asset->take();
	}

	return _asset;
}

template <class C>
AssetManager::CacheEntry<C>::CacheEntry(void) : data(NULL), refs(0) {

}

template <class C>
AssetManager::CacheEntry<C>::~CacheEntry(void) {
	delete data;
}

template <class C>
void AssetManager::CacheEntry<C>::take(void) {
	refs++;
}

template <class C>
void AssetManager::CacheEntry<C>::release(void) {
	if (!--refs) {
		delete data;
		data = NULL;
	}
}

extern AssetManager *gameAssets;
extern TextManager *gameLang;

void selectLanguage(unsigned lang_id);

#endif
