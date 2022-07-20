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

#include <stdexcept>
#include <cstring>
#include "system.h"
#include "lbx.h"

#define LBX_MAGIC 0xfead

#define LANG_GROUPS 6
#define DIPLOMSG_BUFSIZE 200
#define HELP_TITLE_SIZE 80
#define HELP_FILENAME_SIZE 14
#define HELP_TEXT_SIZE 1300
#define HELP_ENTRY_SIZE 1403
#define HELP_INDEX_SIZE 84
#define MSGENG_ENTRY_SIZE 1063

#define ANTARMSG_ARCHIVE "antarmsg.lbx"
#define COUNCMSG_ARCHIVE "councmsg.lbx"
#define RACENAME_ARCHIVE "racename.lbx"
#define SHIPNAME_ARCHIVE "shipname.lbx"
#define STARNAME_ARCHIVE "starname.lbx"
#define RACESTUF_ARCHIVE "racestuf.lbx"
#define TECHNAME_ARCHIVE "techname.lbx"

static const char *misc_archives[TXT_MISC_COUNT] = {"billtext.lbx",
	"billtex2.lbx", "jimtext.lbx", "jimtext2.lbx", "kentext.lbx",
	"kentext1.lbx"};
static const char *maintext_archives[] = {"maintext.lbx", "maingerm.lbx",
	"mainfren.lbx", "mainspan.lbx", "mainital.lbx"};
static const char *eventmsg_archives[] = {"eventmse.lbx", "eventmsg.lbx",
	"eventmsf.lbx", "eventmss.lbx", "eventmsi.lbx"};
static const char *rstring_archives[] = {"rstring0.lbx", "rstring1.lbx",
	"rstring2.lbx", "rstring3.lbx", "rstring4.lbx"};
static const char *credits_archives[] = {"credits.lbx", "gercrdts.lbx",
	"frecrdts.lbx", "spacrdts.lbx", "itacrdts.lbx"};
static const char *skildesc_archives[] = {"skildesc.lbx", "gersklls.lbx",
	"fresklls.lbx", "spasklls.lbx", "itasklls.lbx"};
static const char *techdesc_archives[] = {"techdesc.lbx", "gertecd.lbx",
	"fretecd.lbx", "spatecd.lbx", "itatecd.lbx"};
static const char *diplomsg_archives[] = {"diplomse.lbx", "diplomsg.lbx",
	"diplomsf.lbx", "diplomss.lbx", "diplomsi.lbx"};
static const char *estrings_archives[] = {"estrings.lbx", "estrgerm.lbx",
	"estrfren.lbx", "estrspan.lbx", "estrital.lbx"};
static const char *hstrings_archives[] = {"hestrngs.lbx", "hgstrngs.lbx",
	"hfstrngs.lbx", "hsstrngs.lbx", "histrngs.lbx"};
static const char *help_archives[] = {"help.lbx", "ger_help.lbx",
	"fre_help.lbx", "spa_help.lbx", "ita_help.lbx"};

static LBXArchive *openLBX(const char *filename) {
	char *realname = NULL, *path = NULL;
	LBXArchive *ret;

	realname = findDatadirFile(filename);

	try {
		path = dataPath(realname);
		ret = new LBXArchive(path);
	} catch (...) {
		delete[] realname;
		delete[] path;
		throw;
	}

	delete[] realname;
	delete[] path;
	return ret;
}

static MemoryReadStream *loadLBXAsset(const char *filename,
	unsigned asset_id) {
	LBXArchive *lbx = NULL;
	MemoryReadStream *ret = NULL;

	lbx = openLBX(filename);

	try {
		ret = lbx->loadAsset(asset_id);
	} catch (...) {
		delete lbx;
		throw;
	}

	delete lbx;
	return ret;
}

HelpText::HelpText(void) : title(NULL), text(NULL), archive(NULL), asset_id(0),
	frame(0), section(0), nextParagraph(0) {

}

HelpText::HelpText(const HelpText &other) : title(copystr(other.title)),
	text(copystr(other.text)), archive(copystr(other.archive)),
	asset_id(other.asset_id), frame(other.frame), section(other.section),
	nextParagraph(other.nextParagraph) {

}

HelpText::~HelpText(void) {
	delete[] title;
	delete[] text;
	delete[] archive;
}

const HelpText &HelpText::operator=(HelpText other) {
	char *tmp;

	tmp = title;
	title = other.title;
	other.title = tmp;
	tmp = text;
	text = other.text;
	other.text = tmp;
	tmp = archive;
	archive = other.archive;
	other.archive = tmp;
	asset_id = other.asset_id;
	frame = other.frame;
	section = other.section;
	nextParagraph = other.nextParagraph;
	return *this;
}

HelpLink::HelpLink(void) : title(NULL), id(0) {

}

HelpLink::HelpLink(const HelpLink &other) : title(copystr(other.title)),
	id(other.id) {

}

HelpLink::~HelpLink(void) {
	delete[] title;
}

const HelpLink &HelpLink::operator=(HelpLink other) {
	char *tmp;

	tmp = title;
	title = other.title;
	other.title = tmp;
	id = other.id;
	return *this;
}

LBXArchive::LBXArchive(const char *filename) : _file(), _assetCount(0),
	_index(NULL) {

	unsigned magic, i;
	size_t offset;

	if (!_file.open(filename)) {
		throw std::runtime_error("Cannot open LBX file");
	}

	_assetCount = _file.readUint16LE();
	magic = _file.readUint16LE();

	if (!_assetCount || magic != LBX_MAGIC) {
		throw std::runtime_error("Invalid LBX file");
	}

	_file.readUint32LE();
	offset = _file.readUint32LE();
	_index = new LBXEntry[_assetCount];

	for (i = 0; i < _assetCount; i++) {
		_index[i].offset = offset;
		offset = _file.readUint32LE();

		if (offset <= _index[i].offset) {
			delete[] _index;
			throw std::runtime_error("Invalid LBX entry offset");
		}

		_index[i].size = offset - _index[i].offset;
	}
}

LBXArchive::~LBXArchive(void) {
	delete[] _index;
	_file.close();
}

const char *LBXArchive::filename(void) const {
	return _file.getName();
}

unsigned LBXArchive::assetCount(void) const {
	return _assetCount;
}

MemoryReadStream *LBXArchive::loadAsset(unsigned id) {
	if (id >= _assetCount) {
		throw std::out_of_range("Invalid LBX asset ID");
	}

	_file.seek(_index[id].offset, SEEK_SET);
	return _file.readStream(_index[id].size);
}

TextManager::StringList::StringList(void) : data(NULL), size(0) {

}

TextManager::StringList::~StringList(void) {
	clear();
}

void TextManager::StringList::clear(void) {
	unsigned i;

	for (i = 0; i < size; i++) {
		delete[] data[i];
	}

	delete[] data;
	data = NULL;
	size = 0;
}

const char *TextManager::StringList::operator[](unsigned id) const {
	if (id >= size) {
		throw std::out_of_range("Invalid string ID");
	}

	return data[id];
}

void TextManager::StringList::loadFile(const char *filename, unsigned offset,
	unsigned step, unsigned group_id, unsigned groups) {

	unsigned i, pos, end, newsize;
	LBXArchive *lbx = NULL;
	MemoryReadStream *asset = NULL;

	if (!step || !groups) {
		throw std::logic_error("Invalid LBX text asset grouping");
	}

	lbx = openLBX(filename);
	end = lbx->assetCount() / groups;
	newsize = (end + step - 1) / step;
	pos = group_id * end;
	end += pos;
	pos += offset;
	clear();

	try {
		data = new char*[newsize];
		memset(data, 0, newsize * sizeof(char*));
		size = newsize;

		for (i = 0; pos < end; i++, pos += step) {
			asset = lbx->loadAsset(pos);

			// string count in this asset
			if (asset->readUint16LE() != 1) {
				throw std::runtime_error(
					"Unexpected multitext asset");
			}

			asset->readUint16LE();	// string size, ignore
			data[i] = copystr(asset->readCString());
			delete asset;
			asset = NULL;
		}

		size = i;
	} catch (...) {
		delete asset;
		delete lbx;
		throw;
	}

	delete lbx;
}

void TextManager::StringList::loadAsset(const char *filename,
	unsigned asset_id) {

	unsigned i, newsize, bufsize;
	MemoryReadStream *asset = NULL;
	char *buf = NULL;

	asset = loadLBXAsset(filename, asset_id);
	newsize = asset->readUint16LE();
	bufsize = asset->readUint16LE();

	if (asset->size() < long(newsize * bufsize + 4)) {
		delete asset;
		throw std::runtime_error("Premature end of asset data");
	}

	clear();

	try {
		data = new char*[newsize];
		memset(data, 0, newsize * sizeof(char*));
		size = newsize;
		buf = new char[bufsize + 1];
		buf[bufsize] = '\0';

		for (i = 0; i < size; i++) {
			asset->read(buf, bufsize);
			data[i] = copystr(buf);
		}
	} catch (...) {
		delete[] buf;
		delete asset;
		throw;
	}

	delete[] buf;
	delete asset;
}

void TextManager::StringList::loadStrings(const char *filename,
	unsigned asset_id, unsigned offset) {

	unsigned i, count = 0, newsize = 0;
	MemoryReadStream *asset = NULL;
	const char *str;

	asset = loadLBXAsset(filename, asset_id);
	asset->seek(offset, SEEK_SET);

	// Some assets have empty strings in the middle, scan the whole asset
	// and count all strings up to the last non-empty one
	do {
		str = asset->readCString();
		count++;

		if (str && *str) {
			newsize = count;
		}
	} while (str);

	if (!newsize) {
		return;
	}

	asset->seek(offset, SEEK_SET);
	clear();

	try {
		data = new char*[newsize];
		memset(data, 0, newsize * sizeof(char*));
		size = newsize;

		for (i = 0; i < size; i++) {
			data[i] = copystr(asset->readCString());
		}
	} catch (...) {
		delete asset;
		throw;
	}

	delete asset;
}

TextManager::TextManager(unsigned lang_id) : _diplomsg(NULL),
	_diplomsgCount(0), _help(NULL), _helpCount(0) {

	unsigned i;

	for (i = 0; i < TXT_HELPSECTION_COUNT; i++) {
		_helpIndex[i] = NULL;
		_helpIndexCount[i] = 0;
	}

	load(lang_id);
}

TextManager::~TextManager(void) {
	clear();
}

void TextManager::clear(void) {
	unsigned i;

	for (i = 0; i < TXT_HELPSECTION_COUNT; i++) {
		delete[] _helpIndex[i];
	}

	delete[] _diplomsg;
	delete[] _help;
}

void TextManager::loadDiplomsg(unsigned lang_id) {
	unsigned i, j, size;
	LBXArchive *lbx;
	MemoryReadStream *asset = NULL;
	char buf[DIPLOMSG_BUFSIZE + 1] = {0};

	lbx = openLBX(diplomsg_archives[lang_id]);
	_diplomsgCount = lbx->assetCount();

	try {
		_diplomsg = new StringList[_diplomsgCount];

		for (i = 0; i < _diplomsgCount; i++) {
			asset = lbx->loadAsset(i);

			if (asset->readUint16LE() != 1) {
				throw std::runtime_error(
					"Invalid diplomsg asset");
			}

			asset->readUint16LE();

			if (asset->readUint8() > 1) {
				throw std::runtime_error(
					"Invalid diplomsg format");
			}

			size = asset->readUint8();

			if (!size) {
				delete asset;
				continue;
			}

			if (asset->size() < long(6 + size * DIPLOMSG_BUFSIZE)) {
				throw std::runtime_error(
					"Premature end of asset data");
			}

			_diplomsg[i].data = new char*[size];
			memset(_diplomsg[i].data, 0, size * sizeof(char*));
			_diplomsg[i].size = size;

			for (j = 0; j < size; j++) {
				asset->read(buf, DIPLOMSG_BUFSIZE);
				_diplomsg[i].data[j] = copystr(buf);
			}

			delete asset;
			asset = NULL;
		}
	} catch (...) {
		delete asset;
		delete lbx;
		throw;
	}

	delete lbx;
}

void TextManager::loadHelp(unsigned lang_id) {
	unsigned i, j, count, size;
	LBXArchive *lbx;
	MemoryReadStream *asset = NULL;
	char buf[HELP_TEXT_SIZE + 1] = {0};

	lbx = openLBX(help_archives[lang_id]);

	try {
		asset = lbx->loadAsset(0);
		count = asset->readUint16LE();
		size = asset->readUint16LE();

		if (size < HELP_ENTRY_SIZE) {
			throw std::runtime_error("Invalid help data format");
		}

		if (asset->size() < long(count * size + 4)) {
			throw std::runtime_error("Premature end of asset data");
		}

		_help = new HelpText[count];
		_helpCount = count;

		for (i = 0; i < _helpCount; i++) {
			asset->read(buf, HELP_TITLE_SIZE);
			_help[i].title = copystr(buf);
			asset->read(buf, HELP_FILENAME_SIZE);

			if (*buf) {
				_help[i].archive = copystr(buf);
			}

			_help[i].asset_id = asset->readUint16LE();
			_help[i].frame = asset->readUint16LE();
			_help[i].section = asset->readUint8();
			_help[i].nextParagraph = asset->readUint32LE();
			asset->read(buf, HELP_TEXT_SIZE);
			_help[i].text = copystr(buf);
			asset->seek(size - HELP_ENTRY_SIZE, SEEK_CUR);
		}

		delete asset;
		asset = NULL;

		for (i = 0; i < TXT_HELPSECTION_COUNT; i++) {
			asset = lbx->loadAsset(i + 1);
			count = asset->readUint16LE();
			size = asset->readUint16LE();

			if (size < HELP_INDEX_SIZE) {
				throw std::runtime_error(
					"Invalid help index format");
			}

			if (asset->size() < long(count * size + 4)) {
				throw std::runtime_error(
					"Premature end of asset data");
			}

			_helpIndex[i] = new HelpLink[count];
			_helpIndexCount[i] = count;

			for (j = 0; j < count; j++) {
				asset->read(buf, HELP_TITLE_SIZE);
				_helpIndex[i][j].title = copystr(buf);
				_helpIndex[i][j].id = asset->readUint32LE();
				asset->seek(size - HELP_INDEX_SIZE, SEEK_CUR);
			}

			delete asset;
			asset = NULL;
		}
	} catch (...) {
		delete asset;
		delete lbx;
		throw;
	}

	delete lbx;
}

void TextManager::load(unsigned lang_id) {
	unsigned i;

	for (i = 0; i < TXT_MISC_COUNT; i++) {
		_misctext[i].loadFile(misc_archives[i], lang_id, LANG_GROUPS,
			0, 1);
	}

	_antarmsg.loadFile(ANTARMSG_ARCHIVE, 0, 1, lang_id, LANG_GROUPS);
	_councmsg.loadFile(COUNCMSG_ARCHIVE, 0, 1, lang_id, LANG_GROUPS);
	_maintext.loadFile(maintext_archives[lang_id], 0, 1, 0, 1);
	_eventmsg.loadFile(eventmsg_archives[lang_id], 0, 1, 0, 1);
	_rstring.loadStrings(rstring_archives[lang_id], 0, 4);
	_credits.loadAsset(credits_archives[lang_id], 0);
	_skillname.loadAsset(skildesc_archives[lang_id], 0);
	_skilldesc.loadAsset(skildesc_archives[lang_id], 1);

	for (unsigned i = 0; i < TXT_TECH_COUNT; i++) {
		_techdesc[i].loadAsset(techdesc_archives[lang_id], i);
	}

	_racename.loadAsset(RACENAME_ARCHIVE, 0);
	_shipname.loadAsset(SHIPNAME_ARCHIVE, 0);
	_homeworlds.loadAsset(STARNAME_ARCHIVE, 0);
	_starname.loadAsset(STARNAME_ARCHIVE, 1);
	_estrings.loadStrings(estrings_archives[lang_id], 0, 6);
	_hstrings.loadStrings(hstrings_archives[lang_id], 0, 6);
	_raceTraits.loadStrings(RACESTUF_ARCHIVE, lang_id, 0);
	_raceInfo.loadStrings(RACESTUF_ARCHIVE, 8 + lang_id, 0);
	_techname.loadStrings(TECHNAME_ARCHIVE, lang_id, 0);

	try {
		loadDiplomsg(lang_id);
		loadHelp(lang_id);
	} catch (...) {
		clear();
		throw;
	}
}

const char *TextManager::antarmsg(unsigned str_id) const {
	return _antarmsg[str_id];
}

const char *TextManager::councmsg(unsigned str_id) const {
	return _councmsg[str_id];
}

const char *TextManager::misctext(unsigned file, unsigned str_id) const {
	if (file >= TXT_MISC_COUNT) {
		throw std::out_of_range("Misc text file ID out of range");
	}

	return _misctext[file][str_id];
}

const char *TextManager::maintext(unsigned str_id) const {
	return _maintext[str_id];
}

const char *TextManager::eventmsg(unsigned str_id) const {
	return _eventmsg[str_id];
}

const char *TextManager::rstring(unsigned str_id) const {
	return _rstring[str_id];
}

const char *TextManager::credits(unsigned str_id) const {
	return _credits[str_id];
}

const char *TextManager::skillname(unsigned str_id) const {
	return _skillname[str_id];
}

const char *TextManager::skilldesc(unsigned str_id) const {
	return _skilldesc[str_id];
}

const char *TextManager::techdesc(unsigned asset_id, unsigned str_id) const {
	if (asset_id >= TXT_TECH_COUNT) {
		throw std::out_of_range("Ship tech group ID out of range");
	}

	return _techdesc[asset_id][str_id];
}

const char *TextManager::racename(unsigned str_id) const {
	return _racename[str_id];
}

const char *TextManager::shipname(unsigned str_id) const {
	return _shipname[str_id];
}

const char *TextManager::homeworlds(unsigned str_id) const {
	return _homeworlds[str_id];
}

const char *TextManager::starname(unsigned str_id) const {
	return _starname[str_id];
}

const char *TextManager::estrings(unsigned str_id) const {
	return _estrings[str_id];
}

const char *TextManager::hstrings(unsigned str_id) const {
	return _hstrings[str_id];
}

const char *TextManager::raceTraits(unsigned str_id) const {
	return _raceTraits[str_id];
}

const char *TextManager::raceInfo(unsigned str_id) const {
	return _raceInfo[str_id];
}

const char *TextManager::techname(unsigned str_id) const {
	return _techname[str_id];
}

const char *TextManager::diplomsg(unsigned asset_id, unsigned str_id) const {
	if (asset_id >= _diplomsgCount) {
		throw std::out_of_range("Diplomsg group ID out of range");
	}

	return _diplomsg[asset_id][str_id];
}

const struct HelpText *TextManager::help(unsigned id) const {
	if (id >= _helpCount) {
		throw std::out_of_range("Help entry ID out of range");
	}

	return _help + id;
}

const struct HelpLink *TextManager::helpIndex(unsigned section_id,
	unsigned entry_id) const {

	if (section_id >= TXT_HELPSECTION_COUNT) {
		throw std::out_of_range("Help section ID out of range");
	}

	if (entry_id >= _helpIndexCount[section_id]) {
		throw std::out_of_range("Help index ID out of range");
	}

	return _helpIndex[section_id] + entry_id;
}

AssetManager::AssetManager(void) : _curfile(NULL), _cache(NULL),
	_cacheCount(0), _cacheSize(32), _imgLookupSize(32) {

	_cache = new FileCache[_cacheSize];
	memset(_cache, 0, _cacheSize * sizeof(FileCache));

	try {
		_imageLookup = new CacheEntry<Image>*[_imgLookupSize];
	} catch (...) {
		delete[] _cache;
		throw;
	}

	memset(_imageLookup, 0, _imgLookupSize * sizeof(size_t));
}

AssetManager::~AssetManager(void) {
	size_t i, j;

	for (i = 0; i < _cacheCount; i++) {
		for (j = 0; j < _cache[i].size; j++) {
			delete _cache[i].images[j].data;
		}

		delete[] _cache[i].filename;
		delete[] _cache[i].images;
	}

	delete _curfile;
	delete[] _cache;
	delete[] _imageLookup;
}

AssetManager::FileCache *AssetManager::getCache(const char *filename) {
	size_t i = 0, j = _cacheCount, tmp;
	int dir;
	char *realname;

	while (i < j) {
		tmp = (i + j) / 2;
		dir = strcasecmp(filename, _cache[tmp].filename);

		if (!dir) {
			return _cache + tmp;
		} else if (dir > 0) {
			i = tmp + 1;
		} else {
			j = tmp;
		}
	}

	realname = findDatadirFile(filename);

	if (_cacheCount >= _cacheSize) {
		size_t size = 2 * _cacheSize;
		FileCache *ptr;

		try {
			ptr = new FileCache[size];
		} catch (...) {
			delete[] realname;
			throw;
		}

		memcpy(ptr, _cache, i * sizeof(FileCache));
		memcpy(ptr+i+1, _cache+i, (_cacheCount-i) * sizeof(FileCache));
		delete[] _cache;
		_cache = ptr;
		_cacheSize = size;
	} else {
		for (j = _cacheCount; j > i; j--) {
			_cache[j] = _cache[j - 1];
		}
	}

	_cacheCount++;
	_cache[i].filename = realname;
	_cache[i].size = 0;
	_cache[i].images = NULL;
	return _cache + i;
}

void AssetManager::openArchive(FileCache *entry) {
	LBXArchive *archive;
	char *path;

	if (_curfile && !strcmp(entry->filename, _curfile->filename())) {
		return;
	}

	path = dataPath(entry->filename);

	try {
		archive = new LBXArchive(path);
	} catch (...) {
		delete[] path;
		throw;
	}

	delete[] path;
	delete _curfile;
	_curfile = archive;

	if (!entry->images) {
		size_t size = _curfile->assetCount();

		entry->images = new CacheEntry<Image>[size];
		memset(entry->images, 0, size * sizeof(CacheEntry<Image>));
		entry->size = size;
	}
}

AssetManager::FileCache *AssetManager::cacheImage(const char *filename,
	unsigned id, const uint8_t **palettes, unsigned palcount) {

	FileCache *entry;
	MemoryReadStream *stream;
	Image *img = NULL;
	unsigned texid;

	entry = getCache(filename);

	if (entry->images && id < entry->size && entry->images[id].data) {
		return entry;
	}

	openArchive(entry);

	if (id >= entry->size) {
		throw std::out_of_range("Invalid asset ID");
	}

	stream = _curfile->loadAsset(id);

	try {
		img = new Image(*stream, palettes, palcount);
		texid = img->textureID(0);

		if (texid >= _imgLookupSize) {
			CacheEntry<Image> **lookup = NULL;
			size_t size;

			size = _imgLookupSize;
			size = (2 * size > texid) ? 2 * size : (texid + 1);
			lookup = new CacheEntry<Image>*[size];
			memcpy(lookup, _imageLookup,
				_imgLookupSize * sizeof(*lookup));
			memset(lookup + _imgLookupSize, 0,
				(size - _imgLookupSize) * sizeof(*lookup));
			delete[] _imageLookup;
			_imageLookup = lookup;
			_imgLookupSize = size;
		}
	} catch (...) {
		delete img;
		delete stream;
		throw;
	}


	delete stream;
	entry->images[id].data = img;
	entry->images[id].refs = 0;
	_imageLookup[texid] = entry->images + id;
	return entry;
}

ImageAsset AssetManager::getImage(const char *filename, unsigned id,
	const uint8_t *palette) {
	FileCache *entry = cacheImage(filename, id, &palette, 1);

	return ImageAsset(this, entry->images[id].data);
}

ImageAsset AssetManager::getImage(const char *filename, unsigned id,
	const uint8_t **palettes, unsigned palcount) {
	FileCache *entry = cacheImage(filename, id, palettes, palcount);

	return ImageAsset(this, entry->images[id].data);
}

void AssetManager::takeAsset(const Image *img) {
	unsigned texid;

	if (!img) {
		return;
	}

	texid = img->textureID(0);

	if (texid >= _imgLookupSize || !_imageLookup[texid]) {
		throw std::runtime_error("The image does not belong to this asset manager");
	}

	_imageLookup[texid]->refs++;
}

void AssetManager::freeAsset(const Image *img) {
	unsigned texid;
	CacheEntry<Image> *entry;

	if (!img) {
		return;
	}

	texid = img->textureID(0);

	if (texid >= _imgLookupSize || !_imageLookup[texid]) {
		throw std::runtime_error("The image does not belong to this asset manager");
	}

	entry = _imageLookup[texid];

	if (!--entry->refs) {
		delete entry->data;
		entry->data = NULL;
		_imageLookup[texid] = NULL;
	}
}

MemoryReadStream *AssetManager::rawData(const char *filename, unsigned id) {
	FileCache *entry;

	entry = getCache(filename);
	openArchive(entry);

	if (id >= entry->size) {
		throw std::out_of_range("Invalid asset ID");
	}

	return _curfile->loadAsset(id);
}

void selectLanguage(unsigned lang_id) {
	TextManager *oldlang, *lang = NULL;
	FontManager *oldfonts, *fonts = NULL;

	try {
		lang = new TextManager(lang_id);
		fonts = new FontManager(lang_id);
	} catch (...) {
		delete lang;
		throw;
	}

	oldlang = gameLang;
	oldfonts = gameFonts;
	gameLang = lang;
	gameFonts = fonts;

	if (oldlang) {
		oldlang->discard();
	}

	if (oldfonts) {
		oldfonts->discard();
	}
}
