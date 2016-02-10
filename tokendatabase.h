#ifndef TOKENDATABASE_H
#define TOKENDATABASE_H

#include "clangpluginapi.h"

#include <vector>
#include <wx/thread.h>
#include <wx/string.h>
#include <wx/archive.h>

template<typename _Tp> class ClTreeMap;
class wxString;
typedef int ClFileId;

struct ClAbstractToken
{
    ClAbstractToken() :
        tokenType(ClTokenType_Unknown), fileId(-1), location(ClTokenPosition( 0, 0 )), identifier(), displayName(), scopeName(), tokenHash(0) {}
    ClAbstractToken( ClTokenType typ, ClFileId fId, ClTokenPosition location, wxString name, wxString displayName, wxString scopeName, unsigned tknHash) :
        tokenType(typ), fileId(fId), location(location), identifier(name), displayName(displayName.c_str()), scopeName(scopeName.c_str()), tokenHash(tknHash) {}
    ClAbstractToken( const ClAbstractToken& other ) :
        tokenType(other.tokenType), fileId(other.fileId), location(other.location), identifier(other.identifier), displayName( other.displayName.c_str()), scopeName(other.scopeName.c_str()), tokenHash(other.tokenHash) {}

    static bool ReadIn( ClAbstractToken& token, wxInputStream& in );
    static bool WriteOut( const ClAbstractToken& token,  wxOutputStream& out );

    ClTokenType tokenType;
    ClFileId fileId;
    ClTokenPosition location;
    wxString identifier;
    wxString displayName;
    wxString scopeName;
    unsigned tokenHash;
};

class ClFilenameEntry
{
public:
    ClFilenameEntry( wxString filename, wxDateTime timestamp ) :
        filename(filename),
        timestamp(timestamp) {}
    wxString filename;
    wxDateTime timestamp;
};

class ClFilenameDatabase
{
public:
    ClFilenameDatabase();
    ~ClFilenameDatabase();

    static bool ReadIn( ClFilenameDatabase& tokenDatabase, wxInputStream& in );
    static bool WriteOut( ClFilenameDatabase& db, wxOutputStream& out );

    ClFileId GetFilenameId(const wxString& filename);
    wxString GetFilename(const ClFileId fId);
    wxDateTime GetFilenameTimestamp(const ClFileId fId);
    void UpdateFilenameTimestamp( const ClFileId fId, const wxDateTime timestamp );
private:
    ClTreeMap<ClFilenameEntry>* m_pFileEntries;
    wxMutex m_Mutex;
};

class ClTokenDatabase
{
public:
    ClTokenDatabase( ClFilenameDatabase& fileDB );
    ClTokenDatabase( const ClTokenDatabase& other);
    ~ClTokenDatabase();

    friend void swap( ClTokenDatabase& first, ClTokenDatabase& second );


    static bool ReadIn( ClTokenDatabase& tokenDatabase, wxInputStream& in );
    static bool WriteOut( ClTokenDatabase& tokenDatabase, wxOutputStream& out );

    ClFileId GetFilenameId(const wxString& filename);
    wxString GetFilename(const ClFileId fId);
    wxDateTime GetFilenameTimestamp(const ClFileId fId);
    ClTokenId GetTokenId(const wxString& identifier, ClFileId fId, ClTokenType tokenType, unsigned tokenHash); ///< returns wxNOT_FOUND on failure
    ClTokenId InsertToken(const ClAbstractToken& token); // duplicate tokens are discarded
    ClAbstractToken GetToken(const ClTokenId tId);
    ClFilenameDatabase& GetFileDB()
    {
        return m_FileDB;
    }
    void RemoveToken( const ClTokenId tokenId );
    /**
     * Return a list of tokenId's for the given token identifier
     */
    std::vector<ClTokenId> GetTokenMatches(const wxString& identifier);
    /**
     * Return a list of tokenId's that are found in the given file
     */
    std::vector<ClTokenId> GetFileTokens(const ClFileId fId);

    /**
     * Clears the database
     */
    void Clear();
    /**
     * Shrinks the database by removing all unnecessary elements and memory
     */
    void Shrink();

    /**
     * Updates the data from the argument into the database. This invalidates any token previously present in the database, replacing it by the matching token from the merged-in database.
     */
    void Update( const ClFileId fileId, const ClTokenDatabase& db );
private:
    void UpdateToken( const ClTokenId tokenId, const ClAbstractToken& token);
private:
    ClFilenameDatabase& m_FileDB;
    ClTreeMap<ClAbstractToken>* m_pTokens;
    ClTreeMap<int>* m_pFileTokens;
    wxMutex m_Mutex;
};

#endif // TOKENDATABASE_H
