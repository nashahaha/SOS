cmd_/home/pss/Escritorio/SOS/T7/prodcons.mod := printf '%s\n'   kmutex.o prodcons-impl.o | awk '!x[$$0]++ { print("/home/pss/Escritorio/SOS/T7/"$$0) }' > /home/pss/Escritorio/SOS/T7/prodcons.mod
