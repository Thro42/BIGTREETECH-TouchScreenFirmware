#include "Macros.h"
#include "includes.h"

LISTITEMS printListItems = {
    // title
    LABEL_BACKGROUND,
    // icon                 ItemType    Item Title        item value text(only for custom value)
    {
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACKGROUND, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
        {ICONCHAR_BACK, LIST_LABEL, LABEL_BACKGROUND, LABEL_BACKGROUND},
    }};


// File list number per page
#define NUM_PER_PAGE 5
SCROLL titleScroll;
const GUI_RECT titleRect = {10, (TITLE_END_Y - BYTE_HEIGHT) / 2, LCD_WIDTH - 10, (TITLE_END_Y - BYTE_HEIGHT) / 2 + BYTE_HEIGHT};

SCROLL gcodeScroll;

void macroListDraw(void)
{
  u8 i = 0;

  Scroll_CreatePara(&titleScroll, (u8 *)infoFile.title, &titleRect);
  printListItems.title.address = (u8 *)infoFile.title;
  GUI_SetBkColor(infoSettings.title_bg_color);
  GUI_ClearRect(0, 0, LCD_WIDTH, TITLE_END_Y);
  GUI_SetBkColor(infoSettings.bg_color);

  // folder
  for (i = 0; (i + infoFile.cur_page * NUM_PER_PAGE < infoFile.folderCount) && (i < NUM_PER_PAGE); i++)
  {
    printListItems.items[i].icon = ICONCHAR_FOLDER;
    setDynamicLabel(i, infoFile.folder[i + infoFile.cur_page * NUM_PER_PAGE]);
    printListItems.items[i].titlelabel.index = LABEL_DYNAMIC;
    menuDrawListItem(&printListItems.items[i], i);
  }
  // gcode file
  for (; (i + infoFile.cur_page * NUM_PER_PAGE < infoFile.fileCount + infoFile.folderCount) && (i < NUM_PER_PAGE); i++)
  {
    printListItems.items[i].icon = ICONCHAR_FILE;
    if (infoMachineSettings.long_filename_support == ENABLED && infoFile.source == BOARD_SD)
    {
      setDynamicLabel(i, infoFile.Longfile[i + infoFile.cur_page * NUM_PER_PAGE - infoFile.folderCount]);
    }
    else
    {
      setDynamicLabel(i, infoFile.file[i + infoFile.cur_page * NUM_PER_PAGE - infoFile.folderCount]);
    }
    printListItems.items[i].titlelabel.index = LABEL_DYNAMIC;
    menuDrawListItem(&printListItems.items[i], i);
  }

  //background
  for (; (i < NUM_PER_PAGE); i++)
  {
    printListItems.items[i].icon = ICONCHAR_BACKGROUND;
    printListItems.items[i].titlelabel.index = LABEL_BACKGROUND;
    menuDrawListItem(&printListItems.items[i], i);
  }
  // set page up down button according to page count and current page
  int t_pagenum = (infoFile.folderCount + infoFile.fileCount + (LISTITEM_PER_PAGE - 1)) / LISTITEM_PER_PAGE;
  if ((infoFile.folderCount + infoFile.fileCount) <= LISTITEM_PER_PAGE)
  {
    printListItems.items[5].icon = ICONCHAR_BACKGROUND;
    printListItems.items[6].icon = ICONCHAR_BACKGROUND;
  }
  else
  {
    if (infoFile.cur_page == 0)
    {
      printListItems.items[5].icon = ICONCHAR_BACKGROUND;
      printListItems.items[6].icon = ICONCHAR_PAGEDOWN;
    }
    else if (infoFile.cur_page == (t_pagenum - 1))
    {
      printListItems.items[5].icon = ICONCHAR_PAGEUP;
      printListItems.items[6].icon = ICONCHAR_BACKGROUND;
    }
    else
    {
      printListItems.items[5].icon = ICONCHAR_PAGEUP;
      printListItems.items[6].icon = ICONCHAR_PAGEDOWN;
    }
  }
  menuDrawListItem(&printListItems.items[5], 5);
  menuDrawListItem(&printListItems.items[6], 6);
}

/**************************
 * Menu Macros
 **************************/
void menuMacros(void){
  KEY_VALUES key_num = KEY_IDLE;
  u8 update = 0;

  GUI_Clear(infoSettings.bg_color);
  GUI_DispStringInRect(0, 0, LCD_WIDTH, LCD_HEIGHT, LABEL_LOADING);
  if (scanMacroFilesGcodeFs() == true)
  {
      menuDrawListPage(&printListItems);
      macroListDraw();
  }
  else
  {
      GUI_DispStringInRect(0, 0, LCD_WIDTH, LCD_HEIGHT, (u8 *)requestCommandInfo.cmd_rev_buf);
    Delay_ms(1000);
    infoMenu.cur--;
  }

  while (infoMenu.menu[infoMenu.cur] == menuMacros)
  {
    GUI_SetBkColor(infoSettings.title_bg_color);
    Scroll_DispString(&titleScroll, LEFT);
    GUI_SetBkColor(infoSettings.bg_color);

    key_num = menuKeyGetValue();

    switch (key_num)
    {
    case KEY_ICON_5:
      if (infoFile.cur_page > 0)
      {
        infoFile.cur_page--;
        update = 1;
      }
      break;

    case KEY_ICON_6:
      if (infoFile.cur_page + 1 < (infoFile.folderCount + infoFile.fileCount + (NUM_PER_PAGE - 1)) / NUM_PER_PAGE)
      {
        infoFile.cur_page++;
        update = 1;
      }
      break;

    case KEY_ICON_7:
      infoFile.cur_page = 0;
      if (IsRootDir() == true)
      {
        clearInfoFile();
        infoMenu.cur--;
        break;
      }
      else
      {
        ExitDir();
        scanMacroFilesGcodeFs();
        update = 1;
      }
      break;

    case KEY_IDLE:
      break;

    default:
      if (key_num < ITEM_PER_PAGE)
      {
        u16 start = infoFile.cur_page * NUM_PER_PAGE;
        if (key_num + start < infoFile.folderCount) //folder
        {
          if (EnterDir(infoFile.folder[key_num + start]) == false)
            break;
          scanMacroFilesGcodeFs();
          update = 1;
          infoFile.cur_page = 0;
        }
        else if (key_num + start < infoFile.folderCount + infoFile.fileCount) //gcode
        {
          if (infoHost.connected != true)
            break;
          infoFile.fileIndex = key_num + start - infoFile.folderCount;
          if (EnterDir(infoFile.file[infoFile.fileIndex]) == false)
            break;
          //load model preview in flash if icon exists
          setPrintModelIcon(infoFile.source < BOARD_SD && model_DecodeToFlash(infoFile.title));

          char temp_info[FILE_NUM + 50];
          sprintf(temp_info, (char *)textSelect(LABEL_START_PRINT),
                  (uint8_t *)((infoMachineSettings.long_filename_support == ENABLED && infoFile.source == BOARD_SD) ? infoFile.Longfile[infoFile.fileIndex] : infoFile.file[infoFile.fileIndex]));
          //confirm file selction
          setDialogText(LABEL_PRINT, (u8 *)temp_info, LABEL_CONFIRM, LABEL_CANCEL);
          showDialog(DIALOG_TYPE_QUESTION, startMacro, ExitDir, NULL);
        }
      }
      else if (key_num >= KEY_LABEL_0 && key_num <= KEY_LABEL_4)
      {
      }
      break;
    }

    if (update)
    {
      update = 0;
      macroListDraw();
    }
    loopProcess();
  }
}

/*************************************************/
void startMacro(void)
{
  infoMenu.menu[++infoMenu.cur] = menuExecMacro;
}

void menuExecMacro(void)
{
  uint32_t size = 0;

  request_M98(infoFile.title + 5);
  ExitDir();
  infoMenu.cur--;
  return;
}