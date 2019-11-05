---
title:  "Documentation"
---
# Documentation

### Made By following students of A3

| Roll No | Name|
| -------- | -------- |
| 001710501067| [Dibyajyoti Dhar](https://github.com/djdhar) |
| 001710501070| [Akash Ghosh](https://github.com/Akash-Ghosh-123) |
| 001710501073| [Rohit Rajat Chattopadhyay](https://rohit.chattopadhyay.me) |
| 001710501076| [Priti Shaw](https://github.com/pritishaw) |
| 301810501009| [Soumika Mukherjee](https://github.com/soumiDeb) |


### Table of Contents
1. [Description] (#1-description)
2. [Key Features](#2-key-features)
3. [Important Functions](#3-important-functions)  
4. [Limitations](#4-limitations)
5. [Bug Reporting](#5-bug-reporting)

---
### 1. Description  
To make a text editor, a sequence of characters need to be stored. A cursor is used to help us locate where we want to be editing the sequence of text. This cursor will be between characters, and can be moved using the arrow keys. The cursor allows us to insert new characters at the cursor, and delete characters before the cursor. In a doubly linked list based text editor, each character is considered as a node in the doubly linked list and the insertion and deletion of characters follow the rules of doubly linked list insertion and deletion.

The text editor is designed in C++ using `QtCreator` interface. `QtCreator` provides a GUI environment for the text editor. A `QList` (which is a container class of Qt and behaves as an doubly linked list) is used to store characters. An integer cursor variable is used. Characters are inserted into the list when a key is pressed. The functions like cut, copy and paste are implemented by inserting and removing characters from the list and the cursor variable is incremented or decremented.

---
### 2. Key Features

**Undo and Redo**    
For Undo and Redo operations, stacks are maintained of the cursor and the character list using C++ `QVector` STL. At the end of each operation like entering a character, cut, copy and paste, the new cursor position and character list are pushed back into the stack and gives a snapshot of the operations and modifications done. When an undo operation is encountered, the variables are popped from the stack of operations and pushed into the redo stack. The redo stack stores the cursor position and list of characters after each undo operation. During a redo operation, the variables are popped from the redo stack and pushed into the stack of operations. `Ctrl + Z` is used for UNDO and `Ctrl + Y` is used for REDO.

**Arrow Keys**  
Arrow keys are handled as follows. When `LEFT ARROW` is pressed the cursor is moved one step left. When `RIGHT ARROW` is pressed the cursor is moved one step right. When `DOWN ARROW` is pressed the cursor is moved just right to the next `ENTER` key pressed. When `UP ARROW` is pressed the cursor is moved just left to the previous `ENTER` key pressed.

---
### 3. Important Functions
 
`void on\_actionCut\_triggered()`  
It is used to cut the selected portion of the text. If the start position of the selected text is after the cursor, the selected text is deleted from the start position but the cursor value is not changed. If the end position is before the cursor, the selected text is deleted from the start position and the cursor value is decremented by the length of the selected text.

`void on\_actionCopy\_triggered()`    
It is used to make a copy of the selected portion of the text. The selected text is appended to the list of characters.

`void on\_actionPaste\_triggered()`    
It is used to paste the selected text into the new location pointed to by the cursor. The function first checks whether a cut or copy command precedes the paste command. If true, then the text is added after the cursor and the cursor is incremented by size of selected text.

`void on\_actionSave\_triggered()`    
This function is used to save the file.

`void on\_actionSave\_As\_triggered()`    
This function is used to save the file with a different name.

`void on\_actionOpen\_triggered()`  
This function is used to open the file for writing.

`void on\_actionStatus\_Bar\_toggled(bool)`    
This function shows or hides the status bar.

`void keyPressEvent(QKeyEvent _event)`  
This function is used to handle the exceptions that arise from pressing different keys such as backspace and Enter key.

`void keyReleaseEvent(QKeyEvent _event)`    
This function is used to maintain the row number and column number of the text.

---
### 4. Limitations
The editor supports till `2.7 billion` characters.

---
### 5. Bug Reporting
Found a Bug? Report at [Github](https://github.com/pritishaw/Text-Editor/issues)
