#include "RRFgcode.h"
#include "includes.h"

bool scanMacroFilesGcodeFs(void) {

  clearInfoFile();

  char *ret = request_M20_macros();
  char *data = malloc(strlen(ret) + 1);
  strcpy(data, ret);
  clearRequestCommandInfo();
  char s[3];

  strcpy(s, "\n");

  char *line = strtok(data, s);
  for (; line != NULL; line = strtok(NULL, s))
  {
    if (strcmp(line, "Begin file list") == 0 || strcmp(line, "End file list") == 0 || strcmp(line, "ok") == 0)
      continue; // Start and Stop tag
    if (strlen(line) < strlen(infoFile.title) - 4)
      continue; // No path line exclude
    if (strlen(infoFile.title) > 4 && strstr(line, infoFile.title + 4) == NULL)
      continue; // No current directory

    char *pline = line + strlen(infoFile.title) - 4;
    if (strlen(infoFile.title) > 4)
      pline++;

    if (strchr(pline, '/') == NULL)
    {
      // FILE
      if (infoFile.fileCount >= FILE_NUM)
        continue; /* Gcode max number is FILE_NUM*/

      if (infoMachineSettings.long_filename_support == ENABLED)
      {
        char *Pstr_tmp = strrchr(line, ' ');
        if (Pstr_tmp != NULL)
          *Pstr_tmp = 0;
        //remove file size from line
        char *longfilename;
        if (strrchr(line, '~') != NULL) //check if file name is 8.3 format
          longfilename = request_M33(line);
        else
          longfilename = line;

        Pstr_tmp = strstr(longfilename, "\nok");
        if (Pstr_tmp != NULL)
          *Pstr_tmp = 0; //remove end of M33 command

        Pstr_tmp = strrchr(longfilename, '/'); //remove folder information
        if (Pstr_tmp == NULL)
          Pstr_tmp = longfilename;
        else
          Pstr_tmp++;

        infoFile.Longfile[infoFile.fileCount] = malloc(strlen(Pstr_tmp) + 1);

        if (infoFile.Longfile[infoFile.fileCount] == NULL)
        {
          clearRequestCommandInfo();
          break;
        }
        strcpy(infoFile.Longfile[infoFile.fileCount], Pstr_tmp);
        clearRequestCommandInfo(); // for M33
      }

      char *rest = pline;
      char *file = strtok_r(rest, " ", &rest); //remove file size from pline
      infoFile.file[infoFile.fileCount] = malloc(strlen(file) + 1);
      if (infoFile.file[infoFile.fileCount] == NULL)
        break;
      strcpy(infoFile.file[infoFile.fileCount++], file);
    }
    else
    {
      // DIRECTORY
      if (infoFile.folderCount >= FOLDER_NUM)
        continue; /* floder max number is FOLDER_NUM */

      char *rest = pline;
      char *folder = strtok_r(rest, "/", &rest);

      bool found = false;
      for (int i = 0; i < infoFile.folderCount; i++)
      {
        if (strcmp(folder, infoFile.folder[i]) == 0)
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        uint16_t len = strlen(folder) + 1;
        infoFile.folder[infoFile.folderCount] = malloc(len);
        if (infoFile.folder[infoFile.folderCount] == NULL)
          break;
        strcpy(infoFile.folder[infoFile.folderCount++], folder);
      }
    }
  }
  free(data);
  return true;
}