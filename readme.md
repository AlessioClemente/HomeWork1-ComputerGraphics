# Yocto/Grade: Tiny Color Grading Utilities

Questo homework ha lo scopo principale di abituarci all'utilizzo degli strumenti
di programmazione richiesti dal corso. In particolare, in questo homework
implementeremo quattro funzionalità:

- carimento e salvataggio di immagini,
- tonemapping di immagini,
- color grading di immagini,
- filtri di immagini.

## Framework

Il codice utilizza la libreria [**Yocto/GL**](https://github.com/xelatihy/yocto-gl),
che è inclusa in questo progetto nella directory `yocto`.
Si consiglia di consultare la [**documentazione**](https://xelatihy.github.io/yocto-gl/)
della libreria. Inoltre, dato che la libreria verrà migliorata
durante il corso, consigliamo di mettere **star e watch su github** in modo che
arrivino le notifiche di updates. In particolare, includiamo

- **yocto_math.h**: tipi di base come `vec4f`
- **yocto_color.h**: operazioni sui colori
- **yocto_sampling.h**: generazione di numeri casuali
- **yocto_image.{h,cpp}**: libreria che definisce l'oggetto immagine `image`
  e permette il caricamento e il salvataggio delle immagini
- **yocto_commonio.h**: libreria per lo sviluppo di applicazioni a riga di comando
- **yocto_gui.{h,cpp}**: libreria per lo sviluppo di semplice interfaccie
  utente

Il codice è compilabile attraveso [Xcode](https://apps.apple.com/it/app/xcode/id497799835?mt=12)
su OsX e [Visual Studio 2019](https://visualstudio.microsoft.com/it/vs/) su Windows,
con i tools [cmake](www.cmake.org) e [ninja](https://ninja-build.org)
come mostrato in classe e riassunto, per OsX, nello script `scripts/build.sh`.
Per compilare il codice è necessario installare Xcode su OsX e
Visual Studio 2019 per Windows, ed anche i tools CMake e Ninja.
Come discusso in classe, si consiglia l'utilizzo di
[Visual Studio Code](https://code.visualstudio.com), con i plugins
[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) e
[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools),
che è già predisposto per lavorare su questo progetto.

Questo homework consiste nello sviluppare varie funzionalità che, per semplicità,
sono tutte contenute nella funzione `grade_image()` definita in `yocto_colorgrade.h`
e implementata in `yocto_colorgrade.cpp`. La vostra implementazione va in
`yocto_colorgrade.cpp`. La funzione viene chiamata da `ycolorgrade.cpp` come
interfaccia a riga di comando, e da `ycolorigrades.cpp` che mostra una semplice
interfaccia grafica.

Questo repository contiene anche tests che sono eseguibili da riga di comando
come mostrato in `run.sh`. Le immagini generate dal runner sono depositate
nella directory `out/`. Questi risulati devono combaciare con le immagini nella
directory `check/`.

## Funzionalità (26 punti)

La funzione `grade_image()` applica le seguenti correzioni di colore all'immagine.

- **Tone mapping** applica tre correzioni successive:
  - expoure compensioation: `c = c * 2^exposure`
  - filmic correction: `c *= 0.6; c = (c^2 * 2.51 + c * 0.03) / (c^2 * 2.43 + c * 0.59 + 0.14)`
    che è un fit del tonemapping cinematografico ACES
  - srgb color space: `c = c ^ 1/2.2`
  - clamp result: `c = clamp(c, 0, 1)` con `clamp(c, m, M) = max(min(c, M), m)`
- **Color tint**: `c = c * tint`
- **Saturation**: `c = g + (c - g) * (saturation * 2)` con
  `g = (c.r + c.g + c.b)/3`
- **Contrast**: `c = gain(c, 1 - contrast)` con
  - `gain(a, b) = (a < 0.5) ? bias(a * 2, b) / 2 : bias(a * 2 - 1, 1 - b) / 2 + 0.5`
  - `bias(a, b) = a / ((1 / b - 2) * (1 - a) + 1)`
- **Vignette**: `c = c * (1 - smoothstep(vr, 2 * vr, r))` con
  - `vr = 1 - vignette`
  - `r = length(ij - size/2) / length(size/2)`
  - `size` è la dimensione dell'immagine
  - `ij` le coordinate del pixel
  - `smoothstep(a, b, u) = t * t * (3 - 2 * t)` con
    `t = clamp((u - a) / (b - a), 0, 1)`
- **Film grain**: `c = c + (rand1f(rng) - 0.5) * grain`
  - per generare numeri casuali, usare il generatore `rng_state` creato con
    `make_rng()` e usato con la funzione `rand1f()`
- **Mosaic Effect**: `c[i, j] = c[i - i % mosaic, j - j % mosaic]`
- **Grid Effect**: `c[i, j] = (0 == i % grid || 0 == j % grid) ? 0.5 * c : c`

Le correzioni sono da applicare una dopo l'altra nell'ordine definito sopra
e si applicano solo ai canali RGB dei pixels, che possono essere estratti con
`xyz(pixel)`.

## Extra Credit (8 punti)

Implementare filtri già fatti è onestamente poco interessante. Come extra credit
suggeriamo di creare filtri aggiuntivi. Come ispirazione per altri filtri si
consiglia di cercare su [ShaderToy](https://www.shadertoy.com) o
[The Book of Shaders](https://thebookofshaders.com).

## Istruzioni

Per consegnare l'homework è necessario inviare una ZIP che include il codice e
le immagini generate, cioè uno zip _con le sole directories `libs/yocto_colorgrade` e `out`_.
Per chi fa l'extra credit, includere anche `apps` e ulteriori immagini.
Il file va chiamato `<cognome>_<nome>_<numero_di_matricola>.zip`
e vanno escluse tutte le altre directory. Inviare il file su Google Classroom.
