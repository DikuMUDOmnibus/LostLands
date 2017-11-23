#ROOM * 0 0                            ; Generic Room
#ROOM I 8 0                            ; Indoor Room
#ROOM C 0 1                            ; City Room
#ROOM F 0 2                            ; Field Room
#ROOM T 0 3                            ; Forest Room
#ROOM H 0 4                            ; Hill Room
#ROOM M 0 5                            ; Mountain Room
#ROOM U 0 6                            ; Water (swimmable) Room
#ROOM B 0 7                            ; Water (unswimmable) Room

#PATH | A-B   B-A                      ; Up-Down Passage
#PATH - R-L   L-R                      ; Left-Right Passage
#PATH + A-B   R-L   B-A   L-R          ; Up-Down & Left-Right Passage
#PATH / A-DB  R-DL  B-UA  L-UR         ; Up-Down Passage
#PATH \ A-UB  R-UL  B-DA  L-DR         ; Down-Up Passage
#PATH ^ R-A   B-A   L-A                ; One way passages (no go below)
#PATH > A-R   B-R   L-R                ;                  (no go left)
#PATH v A-B   R-B   L-B                ;                  (no go above)
#PATH < A-L   R-L   B-L                ;                  (no go right)
#PATH @ A-A   R-R   B-B   L-L          ; Circle Passage
#PATH = R-SL  L-SR                     ; "Skip" Left-Right Passage
#PATH r B-R   R-B                      ; Below-Right Corner
#PATH ? B-L   L-B                      ; Below-Left  Corner
#PATH L A-R   R-A                      ; Above-Right Corner
#PATH j A-L   L-A                      ; Above-Left  Corner

#DOOR : 1 -1 door                      ; Normal Open/Close/Lock/Unlock/Pick
#DOOR # 1 -1 gate wooden               ; Normal Open/Close/Lock/Unlock/Pick
