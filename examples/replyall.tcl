bind pubm - * megahal:all

proc megahal:all {nick uhost hand chan text} {
  learn $text
  set reply [getreply $text]
  puthelp "PRIVMSG $chan :$reply"
}
