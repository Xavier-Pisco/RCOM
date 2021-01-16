# Notes on how to use FTP

## FTP COMMANDS

### USER username
Sets the FTP USER to username (on default use 'anonymous').

### PASS password
Sets the PASS for the USER previously set (if user is anonymous type any pass).

### CWD directory
Equivalent to cd directory

### CDUP
Equivalent to cd ..

### REIN
Reinitializes the FTP conection.

### QUIT
Exits FTP conection.

### PASV
Waits for a conection on port that will be displayed.

### RETR
Retrieve a file to the server.

### PWD
Equivalent to pwd

### LIST
Equivalent to ls.

# Net Conection Notes

## Connect eth0
ifconfig eth0 172.16.X0.0/24 -> X = número da bancada (Pode funcionar apenas na sala 1).

## Usefull Commands
- ifconfig [-a]
- netstat [-in]
- netstat [-rn]

# Part2 Experiences

## Experience 1


### Cabos
- T4 -> Switch Console
- T3 -> tux13 S0
- tux13 E0 -> Qualquer porta switch
- tux14 E0 -> Qualquer porta switch

### Comandos

1. ifconfig eth0 up.
2. ifconfig eth0 172.16.X0.Y (X = Número da bancada / Y = Núnmero correspondente ao pc (tux3 = 1, tux4 = 254)).
3. route -n (verificar se o ip que aparece é o do eth0)
   1. IP tux13 = 172.16.10.1
   2. IP tux14 = 172.16.10.254
   3. MAC tux13 = 00:21:5a:61:2d:ef
   3. MAC tux14 = 00:22:64:a6:a4:f8
4. abrir wireshark e capturar os pings


## Experience 2

### Cabos
- tux12 E0 -> Qualquer porta switch

### Comandos
1. Fazer as configurações dos tux12, tux13 e tux14. [Experience 1 Comandos](#comandos).
2. No GTKTerm:
   1. (slide 44)[https://moodle.up.pt/pluginfile.php/36812/mod_resource/content/9/lab2_2020.pdf]
3. abrir wireshark e capturar os pings

## Experience 3

### Cabos

- tux14 E1 -> Switch

### Comandos

1. Configurar tux14 E1
2. Configurar Switch - Remover porta de VLAN
   1. » configure terminal
      » interface fastethernet 0/Porta
      » no switchport access vlan y0
      » end
3. Comandos echo (slide 42)
4. tux14 routes
   1. destination 172.16.10.0 gateway 0.0.0.0
   2. destination 172.16.11.0 gateway 172.16.11.253

## Experience 3

### Cabos

- 1.1 -> RouterGE 0/1
- RouterGE 0/0 -> Switch
- T4 -> RouterConsole

### Comandos

1. GTKTerm
   1. configure terminal
   2. interface gigabitethernet 0/0
   3. ip address 172.16.1.y9 255.255.255.0
   4. exit
2. GTKTerm Switch
   1. configure terminal
   2. interface gigabitethernet 0/1
   3. switchport mode access
   4. switchport access vlan y1
3. Router
   1. conf t 
   2. interface gigabitethernet 0/0 * 
   3. ip address 172.16.y1.254 255.255.255.0 
   4. no shutdown 
   5. ip nat inside 
   6. exit 

   7. interface gigabitethernet 0/1 * 
   8. ip address 172.16.1.y9 255.255.255.0 
   9. no shutdown 
   10. ip nat outside 
   11. exit 

   12. ip nat pool ovrld 172.16.1.y9 172.16.1.y9 prefix 24 
   13. ip nat inside source list 1 pool ovrld overload 

   14. access-list 1 permit 172.16.y0.0 0.0.0.7 
   15. access-list 1 permit 172.16.y1.0 0.0.0.7 

   16. ip route 0.0.0.0 0.0.0.0 172.16.1.254 
   17. ip route 172.16.y0.0 255.255.255.0 172.16.y1.253 
   18. end

   19. * In room I320 use interface fastethernet
4. Comandos echo
   1. echo 0 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
   2. echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects
5. Route delete
   1. route del -net 0.0.0.0 gw 172.16.y0.254
6. Trace Route
   1. traceroute 172.16.y0.1

## Experience 5

### Cabos

### Comandos
   1. nano /etc/resolv.conf
      search netlab.fe.up.pt
      nameserver 172.16.1.1
   2. ping fe.up.pt