#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <clang-c/Index.h>
#include <clang-c/Documentation.h>
#include "clangpluginapi.h"
#include "tokendatabase.h"

#include <map>
#include <algorithm>


unsigned HashToken(CXCompletionString token, wxString& identifier);

struct ClFunctionScope
{
    ClFunctionScope( const wxString& l_functionName, const wxString& l_scopeName, const ClTokenPosition& l_startLocation) :
        functionName(l_functionName),
        scopeName(l_scopeName),
        startLocation(l_startLocation)
    {}
    wxString functionName;
    wxString scopeName;
    ClTokenPosition startLocation;
};

typedef std::vector<ClFunctionScope> ClFunctionScopeList;
typedef std::map<ClFileId, ClFunctionScopeList> ClFunctionScopeMap;

class ClTranslationUnit
{
public:
    ClTranslationUnit(const ClTranslUnitId id);
    ClTranslationUnit(const ClTranslUnitId id, CXIndex clIndex);
    // move ctor
#if __cplusplus >= 201103L
    ClTranslationUnit(ClTranslationUnit&& other);
    ClTranslationUnit(const ClTranslationUnit& other) = delete;
#else
    ClTranslationUnit(const ClTranslationUnit& other);
#endif
    ~ClTranslationUnit();

    /** @brief Swap 2 translation units. Function used mostly to make sure there is only 1 class that manages the Translation Unit resource.
     *
     * @param first The first ClTranslationUnit
     * @param second The second ClTranslationUnit
     * @return friend void
     *
     */
    friend void swap( ClTranslationUnit& first, ClTranslationUnit& second )
    {
        using std::swap;
        assert( first.m_Id == second.m_Id );
        swap(first.m_Id, second.m_Id);
        swap(first.m_FileId, second.m_FileId);
        swap(first.m_Files, second.m_Files);
        swap(first.m_ClIndex, second.m_ClIndex);
        swap(first.m_ClTranslUnit, second.m_ClTranslUnit);
        swap(first.m_LastCC, second.m_LastCC);
        swap(first.m_LastPos.line, second.m_LastPos.line);
        swap(first.m_LastPos.column, second.m_LastPos.column);
        swap(first.m_LastParsed, second.m_LastParsed);
        swap(first.m_FunctionScopes, second.m_FunctionScopes);
    }
    bool UsesClangIndex( const CXIndex& idx )
    {
        return idx == m_ClIndex;
    }

    bool Contains(ClFileId fId)
    {
        return std::binary_search(m_Files.begin(), m_Files.end(), fId);
    }
    int GetFileId() const
    {
        return m_FileId;
    }
    bool IsEmpty() const
    {
        return m_Files.empty();
    }
    bool IsValid() const
    {
        if (IsEmpty())
            return false;
        if (m_ClTranslUnit==nullptr)
            return false;
        if (m_Id < 0)
            return false;
        return true;
    }
    ClTranslUnitId GetId() const
    {
        return m_Id;
    }
    wxDateTime GetLastParsed() const
    {
        return m_LastParsed;
    }

    // note that complete_line and complete_column are 1 index, not 0 index!
    CXCodeCompleteResults* CodeCompleteAt( const wxString& complete_filename, const ClTokenPosition& location,
                                           struct CXUnsavedFile* unsaved_files,
                                           unsigned num_unsaved_files );
    const CXCompletionResult* GetCCResult(unsigned index);
    CXCursor GetTokenAt(const wxString& filename, const ClTokenPosition& location);
    void Parse( const wxString& filename, ClFileId FileId, const std::vector<const char*>& args,
                const std::map<wxString, wxString>& unsavedFiles );
    void Reparse(const std::map<wxString, wxString>& unsavedFiles);
    void ProcessAllTokens(ClTokenDatabase& database, std::vector<ClFileId>& out_includeFileList, ClFunctionScopeMap& out_functionScopes) const;

    void GetDiagnostics(const wxString& filename, std::vector<ClDiagnostic>& diagnostics);
    CXFile GetFileHandle(const wxString& filename) const;
    void ExpandDiagnosticSet(CXDiagnosticSet diagSet, const wxString& filename, std::vector<ClDiagnostic>& diagnostics);
    void ExpandDiagnostic(CXDiagnostic diag, const wxString& filename, std::vector<ClDiagnostic>& diagnostics);

    void SetFiles( const std::vector<ClFileId>& files ){ m_Files = files; }
    void UpdateFunctionScopes( const ClFileId fileId, const ClFunctionScopeList& functionScopes );
    void GetFunctionScopes( const ClFileId fileId, ClFunctionScopeList& out_functionScopes ){ out_functionScopes = m_FunctionScopes[fileId]; }
private:
    ClTranslUnitId m_Id;
    ClFileId m_FileId; ///< The file that triggered the creation of this TU
    std::vector<ClFileId> m_Files; ///< All files linked to this TU
    CXIndex m_ClIndex;
    CXTranslationUnit m_ClTranslUnit;
    CXCodeCompleteResults* m_LastCC;

    struct FilePos
    {
        FilePos(unsigned ln, unsigned col) :
            line(ln), column(col) {}

        void Set(unsigned ln, unsigned col)
        {
            line   = ln;
            column = col;
        }

        bool Equals(unsigned ln, unsigned col)
        {
            return (line == ln && column == col);
        }

        unsigned line;
        unsigned column;
    } m_LastPos;
    bool m_Occupied; // Sentinel flag
    wxDateTime m_LastParsed; // Timestamp when the file was last parsed
    ClFunctionScopeMap m_FunctionScopes;
};

#endif // TRANSLATION_UNIT_H
