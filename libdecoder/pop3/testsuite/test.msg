S:+OK mail.hillstonenet.com
C:USER test1
S:+OK
C:Pass test_0
S:-ErR Auth Failed
C:USER test1
S:+OK
C:Pass test_1
S:-ErR Auth Failed
C:stat
S:-ERR Need Auth
C:user
S:-err no username found
C:retr
S:-err need auth
C:User test2
S:+ok
C:PASS hillstone123
S:+OK Auth Successfully
C:STAT
S:+OK 2 messages(12345 bytes)
C:TOP 1 10
S:+OK 22 bytes
S:2111111111111
S:3222222222222
S:4333333333333
S:5444444444444
S:.
C:List 1
S:+OK 1 12 bytes
C:LIST
S:+OK 2 messages
S:1 22 bytes
S:2 33 bytes
S:.
C:QUIT
S:+OK
