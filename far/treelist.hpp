#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel

*/

/* Revision: 1.03 25.04.2001 $ */

/*
Modify:
  25.04.2001 SVS
    + SetRootDir()
  05.04.2001 VVM
    + ������������ �������� � ����� MACRO_TREEPANEL
  16.10.2000 tran
    + MustBeCached(Root) - �������, ������������ ������������� �����������
      ������
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

class TreeList:public Panel
{
  private:
    int PrevMacroMode;
    void SetMacroMode(int Restore = FALSE);
    void DisplayObject();
    void DisplayTree(int Fast);
    void DisplayTreeName(char *Name,int Pos);
    void Up(int Count);
    void Down(int Count);
    void CorrectPosition();
    static int MsgReadTree(int TreeCount,int &FirstCall);
    void FillLastData();
    int CountSlash(char *Str);
    int SetDirPosition(char *NewDir);
    void GetRoot();
    Panel* GetRootPanel();
    void SyncDir();
    void SaveTreeFile();
    int ReadTreeFile();
    static int GetCacheTreeName(char *Root,char *Name,int CreateDir);
    int GetSelCount();
    int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL);

    struct TreeItem *ListData;
    char Root[NM];
    long TreeCount;
    long WorkDir;
    long GetSelPosition;
    int UpdateRequired;
    int CaseSensitiveSort;
  public:
    TreeList();
    ~TreeList();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
//    void KillFocus();
    void Update(int Mode);
    void ReadTree();
    void SetCurDir(char *NewDir,int ClosePlugin);
    void SetRootDir(char *NewRootDir);
    void GetCurDir(char *CurDir);
    int GetCurName(char *Name,char *ShortName);
    void UpdateViewPanel();
    void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int FindPartName(char *Name,int Next);
    int GoToFile(char *Name);
    void ProcessEnter();
    static void AddTreeName(char *Name);
    static void DelTreeName(char *Name);
    static void RenTreeName(char *SrcName,char *DestName);
    static void ReadSubTree(char *Path);
    static void ClearCache(int EnableFreeMem);
    static void ReadCache(char *TreeRoot);
    static void FlushCache();

    /* $ 16.10.2000 tran
       �������, ������������� ������������� �����������
       ����� */
    static int MustBeCached(char *Root);
    /* tran $ */
    virtual void SetFocus();
    virtual void KillFocus();
};

#endif  // __TREELIST_HPP__
