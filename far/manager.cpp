/*
manager.cpp

������������ ����� ����������� file panels, viewers, editors, dialogs

*/

/* Revision: 1.29 04.06.2001 $ */

/*
Modify:
  04.06.2001 OT
     �������� ��� "�������������" ���������� ������
  30.05.2001 OT
    - ��� ���� memory leak ����� F6 � ���������/������. ���������� ������� UpdateCommit()
    - ���������� CloseAll() � ExitAll() � �������� NFZ.
    - ����������� ActivateCommit(). ��� ��������� ���������������
      �������� "�������������" ActivatedFrame.
    + AltF9 ������� ������ �� ������ � �������, �� � ... ����� :)
  28.05.2001 OT
    - ����������� "���� ����� F3 �������� ����������" (��������� DeleteCommit())
  26.05.2001 OT
    - ����������� ExucuteModal()
    + ����� ������ ExecuteComit(), ExecuteFrame(), IndexOfStack()
    + ����� ���� Frame *ExecutedFrame;
    ! ����������� ������� DeleteCommit(), UpdateCommit(), ��������� � ���������� ExecuteFrame
    ! ����������� ��������� RefreshCommit() � ������ ����������.
  25.05.2001 DJ
    - ��������� ���� ��� �������� Alt-F7.
  23.05.2001 OT
    - ����������� ���� - �������� Frame, �� ���������� � ������ FrameList
  22.05.2001 OT
    + ��������� RefreshedFrame
  22.05.2001 DJ
    ! � ExecuteModal() ������ ����� ������ ����� commit (���� ��������
      ��������� ��������, �������� ������ �����)
  21.05.2001 DJ
    ! ������ �������������; � ����� � ���������� ������ ���� �������
      �������� GetFrameTypesCount(); �� ���������� OnChangeFocus(0)
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      ���� Selected, Checked, Separator � Disabled ������������� � DWORD Flags
    ! ��������� MENU_ - � ����
  16.05.2001 DJ
    ! ����������� ExecuteModal()
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    - ������ � F4 -> ReloadAgain
  12.05.2001 DJ
    ! ����� ModalFrame.Show() � Manager::ExecuteModal()
  12.05.2001 DJ
    ! FrameManager ������� �� CtrlObject
    ! ���������� ExecuteModal() � ExecuteModalPtr() (� � ��� � �����, �����
      ����� ��� �������?)
    ! ReplaceCurrentFrame() �������� �� ����� ������������� ReplaceFrame()
      (� ������ ��)
  11.05.2001 OT
    ! ��������� Background
  10.05.2001 DJ
    + SwitchToPanels()
    * GetFrameTypesCount() �� ��������� �����, ������� �� ��������� �������
    + ModalStack
    - ������ ��������� ������ DestroyFrame() � ��� � ����
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    ! ��������� � ������� CloseAll() � ExitAll()
  06.05.2001 DJ
    ! �������� #include
    + ReplaceCurrentFrame()
  07.05.2001 ��
    - ��� � �������� ������� �������� ������ FramePos ��� ��������
      ������-������ �� ������ :)
  06.05.2001 ��
    ! �������������� Window � Frame :)
  05.05.2001 DJ
    + �������� NWZ
  04.05.2001 OT
    + ������� ������������� ���� �������� �� F11 (NWZ)
      ��������� PluginSet::CommandsMenu()
  29.04.2001 ��
    + ��������� NWZ �� ����������
  29.12.2000 IS
    + ����� ExitAll - ������ CloseAll, �� ��������� ����������� �����������
      ������ � ����, ���� ������������ ��������� ������������� ����.
      ���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
  28.07.2000 tran 1.04
    + ��������� ��� ������ ������ ���� -
      ���������� ����� � ��������� ����������� "*"
  13.07.2000 SVS
    ! ��������� ��������� ��� ������������� new/delete/realloc
  11.07.2000 SVS
    ! ��������� ��� ����������� ���������� ��� BC & VC
  28.06.2000 tran
    - NT Console resize
      add class member ActiveModal
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "headers.hpp"
#pragma hdrstop

#include "manager.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

Manager *FrameManager;

Manager::Manager()
{
  FrameList=NULL;
  FrameCount=FramePos=FrameListSize=0;
  ModalStack=NULL;
  ModalStackSize = ModalStackCount = 0;
  EndLoop = FALSE;
  RefreshedFrame=NULL;

  CurrentFrame  = NULL;
  InsertedFrame = NULL;
  DeletedFrame  = NULL;
  ActivatedFrame= NULL;
  DeactivatedFrame=NULL;
  ModalizedFrame=NULL;
  ExecutedFrame=NULL;

  DisableDelete = FALSE;
}

Manager::~Manager()
{
  if (FrameList)
    free(FrameList);
  if (ModalStack)
    free (ModalStack);
}


/* $ 29.12.2000 IS
  ������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
  ���� ������������ ��������� ������������� ����.
  ���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
*/
BOOL Manager::ExitAll()
{
  bool EnterFlag=false;
  int i;
  do {
    bool NoSavedFile=false;
    int PrevFrameCount=FrameCount;
    for (i=FrameCount-1; i>=1; i--,EnterFlag=false){
      Frame *iFrame=FrameList[i];
      if (!iFrame->GetCanLoseFocus(TRUE)){
        NoSavedFile=true;
        ActivateFrame(iFrame);
        Commit();
        iFrame->ProcessKey(KEY_ESC);
        Commit();
        EnterFlag=true;
        break;
      }
    }
    if (NoSavedFile && PrevFrameCount==FrameCount){
      return FALSE;
    }
  } while (EnterFlag);
  return TRUE;
}
/* IS $ */

void Manager::CloseAll()
{
  int i;
  Frame *iFrame;
  for (i=ModalStackCount-1;i>=0;i--){
    iFrame=ModalStack[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  for (i=FrameCount-1;i>=0;i--){
    iFrame=(*this)[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  /* $ 13.07.2000 SVS
     ����� ���� "delete ModalList;", �� ����������������� ������� ������
     ���� ����� realloc...
  */
  free(FrameList);
  /* SVS $ */
  FrameList=NULL;
  FrameCount=FramePos=0;
}

BOOL Manager::IsAnyFrameModified(int Activate)
{
  for (int I=0;I<FrameCount;I++)
    if (FrameList[I]->IsFileModified())
    {
      if (Activate)
      {
        ActivateFrame(I);
        Commit();
      }
      return(TRUE);
    }

  return(FALSE);
}

void Manager::InsertFrame(Frame *Inserted, int Index)
{
  _OT(SysLog("InsertFrame(), Inserted=%p, Index=%i",Inserted, Index));
  if (Index==-1)
    Index=FramePos;
  InsertedFrame=Inserted;
}

void Manager::DeleteFrame(Frame *Deleted)
{
  _OT(SysLog("DeleteFrame(), Deleted=%p",Deleted));
  if (!Deleted){
    DeletedFrame=CurrentFrame;
  } else {
    DeletedFrame=Deleted;
  }
}

void Manager::DeleteFrame(int Index)
{
  _OT(SysLog("DeleteFrame(), Index=%i",Index));
  DeleteFrame(this->operator[](Index));
}


void Manager::ModalizeFrame (Frame *Modalized, int Mode)
{
  _OT(SysLog("ModalizeFrame(), Modalized=%p",Modalized));
  ModalizedFrame=Modalized;
}

void Manager::ExecuteModal (Frame *Executed)
{
  _OT(SysLog("ExecuteModal(), Executed=%p, ExecutedFrame=%p",Executed,ExecutedFrame));
  if (!Executed && !ExecutedFrame){
    return;
  }
  if (Executed){
    if (ExecutedFrame) {
      _OT(SysLog("������� � ����� ����� ��������� � ��������� ������ ��� ������. Executed=%p, ExecitedFrame=%p",Executed, ExecutedFrame));
      return;// NULL; //?? ����������, ����� �������� ��������� ���������� � ���� ������
    } else {
      ExecutedFrame=Executed;
    }
  }

  int ModalStartLevel=ModalStackCount;
  while (1){
    Commit();
    if (ModalStackCount<=ModalStartLevel){
      break;
    }
    ProcessMainLoop();
  }
  return;// GetModalExitCode();
}

int Manager::GetModalExitCode()
{
  return ModalExitCode;
}


void Manager::FrameMenu()
{
  int ExitCode;
  {
    struct MenuItem ModalMenuItem;
    memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
    VMenu ModalMenu(MSG(MScreensTitle),NULL,0,ScrY-4);
    ModalMenu.SetHelp("ScrSwitch");
    ModalMenu.SetFlags(VMENU_WRAPMODE);
    ModalMenu.SetPosition(-1,-1,0,0);

    for (int I=0;I<FrameCount;I++)
    {
      char Type[200],Name[NM],NumText[100];
      FrameList[I]->GetTypeAndName(Type,Name);
      if (I<10)
        sprintf(NumText,"&%d. ",I);
      else
        strcpy(NumText,"&   ");
      /* $ 28.07.2000 tran
         ���� ������� �� ������ ������ */
      TruncPathStr(Name,ScrX-40);
      /*  ����������� "*" ���� ���� ������� */
      sprintf(ModalMenuItem.Name,"%s%-20s %c %s",NumText,Type,(FrameList[I]->IsFileModified()?'*':' '),Name);
      /* tran 28.07.2000 $ */
      ModalMenuItem.SetSelect(I==FramePos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    ModalMenu.Process();
    ExitCode=ModalMenu.GetExitCode();
  }
  if (ExitCode>=0)
  {
    ActivateFrame (ExitCode);
  }
}


int Manager::GetFrameCountByType(int Type)
{
  int ret=0;
  for (int I=0;I<FrameCount;I++)
  {
    /* $ 10.05.2001 DJ
       �� ��������� �����, ������� ���������� �������
    */
    if (FrameList[I] == DeletedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
      continue;
    /* DJ $ */
    if (FrameList[I]->GetType()==Type)
      ret++;
  }
  return ret;
}

void Manager::SetFramePos(int NewPos)
{
  _D(SysLog("Manager::SetFramePos(), NewPos=%i",NewPos));
  FramePos=NewPos;
}

/*$ 11.05.2001 OT ������ ����� ������ ���� �� ������ �� ������� �����, �� � �������� - ����, �������� ��� */
int  Manager::FindFrameByFile(int ModalType,char *FileName,char *Dir)
{
  char bufFileName[NM];
  char *FullFileName=FileName;
  if (Dir){
    strcpy(bufFileName,Dir);
    AddEndSlash(bufFileName);
    strcat(bufFileName,FileName);
    FullFileName=bufFileName;
  }
  for (int I=0;I<FrameCount;I++)
  {
    char Type[200],Name[NM];
    if (FrameList[I]->GetTypeAndName(Type,Name)==ModalType)
      if (LocalStricmp(Name,FullFileName)==0)
        return(I);
  }
  return(-1);
}
/* 11.05.2001 OT $*/

void Manager::ShowBackground()
{
  if (!RegVer)
  {
    Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
    return;
  }
  CtrlObject->CmdLine->ShowBackground();
}


void Manager::ActivateFrame(Frame *Activated)
{
  _OT(SysLog("ActivateFrame(), Activated=%i",Activated));
  ActivatedFrame=Activated;
}

void Manager::ActivateFrame(int Index)
{
  _OT(SysLog("ActivateFrame(), Index=%i",Index));
  ActivatedFrame=this->operator[](Index);
}

void Manager::DeactivateFrame (Frame *Deactivated,int Direction)
{
  _OT(SysLog("DeactivateFrame(), Deactivated=%p",Deactivated));
  FramePos+=Direction;
  if (Direction>0){
    if (FramePos>=FrameCount){
      FramePos=0;
    }
  } else {
    if (FramePos<0) {
      FramePos=FrameCount-1;
    }
  }
  ActivateFrame(FramePos);
}

void Manager::RefreshFrame(Frame *Refreshed)
{
  _OT(SysLog("RefreshFrame(), Refreshed=%p",Refreshed));
  if (ActivatedFrame)
    return;
  if (Refreshed){
    RefreshedFrame=Refreshed;
  } else {
    RefreshedFrame=CurrentFrame;
  }
}

void Manager::RefreshFrame(int Index)
{
  RefreshFrame((*this)[Index]);
}

void Manager::ExecuteFrame(Frame *Executed, int DynamicallyBorn)
{
  _OT(SysLog("ExecuteFrame(), Executed=%p, DynamicallyBorn=%i",Executed,DynamicallyBorn));
  Executed->SetDynamicallyBorn(DynamicallyBorn);
  ExecutedFrame=Executed;
}


/* $ 10.05.2001 DJ
   ������������� �� ������ (����� � ������� 0)
*/

void Manager::SwitchToPanels()
{
  ActivateFrame (0);
}

/* DJ $ */

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  while (!EndLoop)
  {
    ProcessMainLoop();
    Commit();
  }
}


void Manager::ProcessMainLoop()
{

  WaitInMainLoop=IsPanelsActive();

  WaitInFastFind++;
  int Key=GetInputRecord(&LastInputRecord);
  WaitInFastFind--;
  WaitInMainLoop=FALSE;
  if (EndLoop)
    return;
  if (LastInputRecord.EventType==MOUSE_EVENT)
    ProcessMouse(&LastInputRecord.Event.MouseEvent);
  else
    ProcessKey(Key);
}

void Manager::ExitMainLoop(int Ask)
{
  if (!Ask || !Opt.Confirm.Exit || Message(0,2,MSG(MQuit),MSG(MAskQuit),MSG(MYes),MSG(MNo))==0)
   /* $ 29.12.2000 IS
      + ���������, ��������� �� ��� ���������� �����. ���� ���, �� �� �������
        �� ����.
   */
   if(ExitAll())
   /* IS $ */
    if (!CtrlObject->Cp()->LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !CtrlObject->Cp()->RightPanel->ProcessPluginEvent(FE_CLOSE,NULL))
      EndLoop=TRUE;
}

int  Manager::ProcessKey(int Key)
{
  int ret=FALSE;
  _D(char kn[32]);
  _D(KeyToText(Key,kn));
  //    _D(SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn));

  if ( CurrentFrame)
  {
    //      _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
    int i;
    switch(Key)
    {
    case KEY_CONSOLE_BUFFER_RESIZE:
      for (i=0;i<FrameCount;i++){
        FrameList[i]->ResizeConsole();
      }
      for (i=0;i<ModalStackCount;i++){
        ModalStack[i]->ResizeConsole();
      }
      FrameManager->RefreshFrame();
      return TRUE;
    case KEY_F11:
      PluginsMenu();
      FrameManager->RefreshFrame();
      _D(SysLog(-1));
      return TRUE;
    case KEY_F12:
      if (CurrentFrame->GetCanLoseFocus())
        FrameMenu();
      _D(SysLog(-1));
      return TRUE;
    case KEY_CTRLTAB:
      if (CurrentFrame->GetCanLoseFocus())
        DeactivateFrame(CurrentFrame,1);
        _D(SysLog(-1));
      return TRUE;
    case KEY_CTRLSHIFTTAB:
      if (CurrentFrame->GetCanLoseFocus())
        DeactivateFrame(CurrentFrame,-1);
      _D(SysLog(-1));
      return TRUE;
    case KEY_ALTF9:
      _OT(SysLog("Manager::ProcessKey, KEY_ALTF9 pressed..."));
      SetVideoMode(FarAltEnter(-2));
      return TRUE;
    }
    CurrentFrame->UpdateKeyBar();
    CurrentFrame->ProcessKey(Key);
  }
  _D(SysLog(-1));
  return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
    int ret=FALSE;
//    _D(SysLog(1,"Manager::ProcessMouse()"));
    if ( CurrentFrame)
        ret=CurrentFrame->ProcessMouse(MouseEvent);
//    _D(SysLog("Manager::ProcessMouse() ret=%i",ret));
    _D(SysLog(-1));
    return ret;
}

void Manager::PluginsMenu()
{
  _D(SysLog(1));
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
    CtrlObject->Plugins.CommandsMenu(curType,0,0);
  _D(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
  return CurrentFrame?CurrentFrame->GetType() == MODALTYPE_PANELS:FALSE;
}

Frame *Manager::operator[](int Index)
{
  if (Index<0||Index>=FrameCount){
    return NULL;
  }
  return FrameList[Index];
}

int Manager::IndexOfStack(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<ModalStackCount;i++)
  {
    if (Frame==ModalStack[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

int Manager::IndexOf(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<FrameCount;i++)
  {
    if (Frame==FrameList[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

BOOL Manager::Commit()
{
  _OT(SysLog(1));
  int Result = false;
  if (DeletedFrame && (InsertedFrame||ModalizedFrame||ExecutedFrame)){
    UpdateCommit();
    DeletedFrame = NULL;
    InsertedFrame = NULL;
    ModalizedFrame = NULL;
    ExecutedFrame=NULL;
    Result=true;
  } else if (ExecutedFrame) {
    ExecuteCommit();
    ExecutedFrame=NULL;
    Result=true;
  } else if (DeletedFrame){
    DeleteCommit();
    DeletedFrame = NULL;
    Result=true;
  } else if (InsertedFrame||ModalizedFrame){
    InsertCommit();
    InsertedFrame =
    ModalizedFrame = NULL;
    Result=true;
  } else if(ActivatedFrame||DeactivatedFrame){
    ActivateCommit();
    DeactivatedFrame=NULL;
    ActivatedFrame=NULL;
    Result=true;
  } else if (RefreshedFrame){
    RefreshCommit();
    RefreshedFrame=NULL;
    Result=true;
  }
  if (Result){
    Result=Commit();
  }
  _OT(SysLog(-1));
  return Result;
}

void Manager::ActivateCommit()
{
  _OT(SysLog("ActivateCommit(), ActivatedFrame=%p, DeactivatedFrame=%p",ActivatedFrame,DeactivatedFrame));
  if (DeactivatedFrame){
    DeactivatedFrame->OnChangeFocus(0);
  }
  if (CurrentFrame==ActivatedFrame){
    RefreshedFrame=ActivatedFrame;
    return;
  }
  int FrameIndex=IndexOf(ActivatedFrame);
  if (-1!=FrameIndex){
    FramePos=FrameIndex;
    int ModalTopIndex=ActivatedFrame->ModalCount();
    if (ModalTopIndex>0)
    {
      ActivatedFrame->OnChangeFocus(TRUE);
      for (int i=0;i<ModalTopIndex-1;i++){
        Frame *iModal=(*ActivatedFrame)[i];
        iModal->OnChangeFocus(TRUE);
      }
      CurrentFrame=(*ActivatedFrame)[ModalTopIndex-1];
    } else {
      RefreshedFrame=CurrentFrame=ActivatedFrame;
    }
  } else if (ModalStackCount) {
    if (ModalStack[ModalStackCount-1]==ActivatedFrame){
      _OT(SysLog("[%p] Top Modal frame",ActivatedFrame));
      RefreshedFrame=CurrentFrame=ActivatedFrame;
    }
  } else {
    Frame *FoundFrame=NULL;
    for (int i=0;i<FrameCount;i++){
      Frame *iFrame=FrameList[i];
      int TopModalIndex=(*iFrame)[ActivatedFrame];
      if((TopModalIndex>=0)&&(TopModalIndex==iFrame->ModalCount()-1)){
        FoundFrame=ActivatedFrame;
        break;
      }
    }
    if (FoundFrame){
      CurrentFrame=FoundFrame;
    }
  }
  /* DJ $ */
}

void Manager::UpdateCommit()
{
  _OT(SysLog("UpdateCommit(), DeletedFrame=%p, InsertedFrame=%p, ExecutedFrame=%p",DeletedFrame,InsertedFrame, ExecutedFrame));
  if (ExecutedFrame){
    DeleteCommit();
    ExecuteCommit();
    return;
  }
  int FrameIndex=IndexOf(DeletedFrame);
  if (-1!=FrameIndex){
    ActivatedFrame=FrameList[FrameIndex] = InsertedFrame;
    DeleteCommit();
  } else {
    Frame *iFrame=NULL;
    Frame *FoundModal=NULL;
    for (int i=0;i<FrameCount;i++){
      iFrame=FrameList[i];
      int ModalDeletedIndex=(*iFrame)[DeletedFrame];
      if(ModalDeletedIndex>=0){
        FoundModal=iFrame;
        if (ModalDeletedIndex>0){
          int iModalCount=iFrame->ModalCount();
          for (int i=iModalCount-1;i>ModalDeletedIndex;i--){
            iFrame->Pop();
          }
        } else {
          iFrame->DestroyAllModal();
        }
        iFrame->Push(ModalizedFrame);
        break;
      }
    }
    if (FoundModal){
      ActivatedFrame = (*iFrame)[iFrame->ModalCount()-1];
    } else {
      DeleteCommit();
    }
  }
}

void Manager::DeleteCommit()
{
  _OT(SysLog("DeleteCommit(), DeletedFrame=%p",DeletedFrame));
  if (!DeletedFrame){
    return;
  }
  if (ModalStackCount&&(DeletedFrame==ModalStack[ModalStackCount-1])){
    ModalStackCount--;
    if (ModalStackCount){
      ActivatedFrame=ModalStack[ModalStackCount-1];
    } else {
      ActivatedFrame=FrameList[FramePos];
    }
  } else {
    int FrameIndex=IndexOf(DeletedFrame);
    if (-1!=FrameIndex){
      DeletedFrame->DestroyAllModal();
      for (int j=FrameIndex; j<FrameCount-1; j++ ){
        FrameList[j]=FrameList[j+1];
      }
      FrameCount--;
      if ( FramePos>=FrameCount ){
        FramePos=0;
      }
      ActivatedFrame=FrameList[FramePos];
    } else {
      for (int i=0;i<FrameCount;i++){
        Frame *iFrame=FrameList[i];
        int ModalDeletedIndex=(*iFrame)[DeletedFrame];
        if(ModalDeletedIndex>=0){
          if (ModalDeletedIndex>0){
            int iModalCount=iFrame->ModalCount();
            for (int j=iModalCount;j>ModalDeletedIndex;j--){
              iFrame->Pop();
            }
            ActivatedFrame = (*iFrame)[iFrame->ModalCount()-1];
          } else {
            ActivatedFrame = iFrame;
            iFrame->DestroyAllModal();
          }
          break;
        }
      }
    }
    if (DeletedFrame->GetDynamicallyBorn()){
      DeletedFrame->OnDestroy();
      delete DeletedFrame;
    }
  }
}

void Manager::InsertCommit()
{
  _OT(SysLog("InsertCommit(), InsertedFrame=%p",InsertedFrame));
  if (ModalizedFrame){
    int FrameIndex=IndexOf(CurrentFrame);
    if (FrameIndex==-1) {
      for (int i=0;i<FrameCount;i++){
        Frame *iFrame=FrameList[i];
        if ((*iFrame)[CurrentFrame]!=-1){
          iFrame->Push(ModalizedFrame);
          break;
        }
      }
    } else {
      CurrentFrame->Push(ModalizedFrame);
    }
    ActivatedFrame=ModalizedFrame;
  } else if (InsertedFrame){
    if (FrameListSize <= FrameCount)
    {
      FrameList=(Frame **)realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
      FrameListSize++;
    }
    FrameList[FrameCount]=InsertedFrame;
    if (!ActivatedFrame){
      ActivatedFrame=InsertedFrame;
    }
    FrameCount++;
  }
}

void Manager::RefreshCommit()
{
  _OT(SysLog("RefreshCommit(), RefreshedFrame=%p,Refreshable()=%i",RefreshedFrame,RefreshedFrame->Refreshable()));
  if (!RefreshedFrame)
    return;
  if (RefreshedFrame->Refreshable()){
    RefreshedFrame->ShowConsoleTitle();
    RefreshedFrame->OnChangeFocus(1);
    CtrlObject->Macro.SetMode(RefreshedFrame->GetMacroMode());
  }
}

void Manager::ExecuteCommit()
{
  _OT(SysLog("ExecuteCommit(), ExecutedFrame=%p",ExecutedFrame));
  if (!ExecutedFrame) {
    return;
  }
  if (ModalStackCount == ModalStackSize){
    ModalStack = (Frame **) realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  }
  ModalStack [ModalStackCount++] = ExecutedFrame;
  ActivatedFrame=ExecutedFrame;
}
