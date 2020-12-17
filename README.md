# Signature-RSA-ARM

### Faire un bootloader:
https://github.com/viktorvano/STM32-Bootloader

### Ecrire l'app PC qui envoie discute avec le bootloader pour flash
##### scripte python qui:
  - lance le flash (erase + start flash)
  - send packet de 4bytes (32bits)
  - arrete le flash (end flash)
  - erase la flash (erase flash)

### Ecrire l'app de gestionnaire de mot de passe

Pour check la signature RSA, le binaire qu'on va recevoir devra avoir le hash inclu dans un structure que l'on pourra check a la mano. Si le check est bon on fait un jump sur le code a executer sinon on leave ou on attend un nouveau binaire.
