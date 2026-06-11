#ifndef REPLIES_HPP
#define REPLIES_HPP

// ============================================================
// RPL — Respuestas de registro y bienvenida
// ============================================================
#define RPL_WELCOME             1
#define RPL_YOURHOST            2
#define RPL_CREATED             3
#define RPL_MYINFO              4

// ============================================================
// RPL — NICK
// ============================================================
#define RPL_NICKLIST            353
#define RPL_ENDOFNAMES          366

// ============================================================
// ERR — Registro
// ============================================================
#define ERR_NONICKNAMEGIVEN     431
#define ERR_ERRONEUSNICKNAME    432
#define ERR_NICKNAMEINUSE       433
#define ERR_NEEDMOREPARAMS      461
#define ERR_ALREADYREGISTRED    462
#define ERR_PASSWDMISMATCH      464

// ============================================================
// ERR — Permisos generales
// ============================================================
#define ERR_UNKNOWNCOMMAND      421
#define ERR_NOTREGISTERED       451
#define ERR_NOPRIVILEGES        481

#endif