bind pubm - "* *$nick*" megahal:rplyall

proc megahal:rplyall {nick uhost hand chan text} {
  learn $text
  set reply [getreply $text]
  puthelp "PRIVMSG $chan :$reply"
}
