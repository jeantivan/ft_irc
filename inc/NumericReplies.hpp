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
// RPL — Canal
// ============================================================
#define RPL_TOPIC           	332
#define RPL_NOTOPIC         	331
#define RPL_NAMREPLY			353			


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

// ============================================================
// ERR — Canal
// ============================================================
#define ERR_NOSUCHCHANNEL   	403
#define ERR_CHANNELISFULL   	471
#define ERR_INVITEONLYCHAN  	473
#define ERR_BADCHANNELKEY   	475
#define ERR_BADCHANNAME			479
#define ERR_NOTONCHANNEL        442

// ============================================================
// ERR — Privmsg
// ============================================================
#define ERR_NORECIPIENT			411
#define ERR_NOTEXTTOSEND		412
#define ERR_NOSUCHNICK			401
#define ERR_CANNOTSENDTOCHAN	404

// ============================================================
// ERR — Ping/Pong
// ============================================================
#define ERR_NOORIGIN			409
#define ERR_NOSUCHSERVER		402

#endif