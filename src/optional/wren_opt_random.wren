foreign class Random {
  construct new() {
    seed_()
  }

  construct new(seed) {
    if (seed is Num) {
      seed_(seed)
    } else if (seed is Sequence) {
      if (seed.isEmpty) Fiber.abort("Sequence cannot be empty.")

      // TODO: Secuencia vacía.
      var seeds = []
      for (element in seed) {
        if (!(element is Num)) Fiber.abort("Sequence elements must all be numbers.")

        seeds.add(element)
        if (seeds.count == 16) break
      }

      // Cicla los valores para completar cualquier hueco faltante.
      var i = 0
      while (seeds.count < 16) {
        seeds.add(seeds[i])
        i = i + 1
      }

      seed_(
          seeds[0], seeds[1], seeds[2], seeds[3],
          seeds[4], seeds[5], seeds[6], seeds[7],
          seeds[8], seeds[9], seeds[10], seeds[11],
          seeds[12], seeds[13], seeds[14], seeds[15])
    } else {
      Fiber.abort("Seed must be a number or a sequence of numbers.")
    }
  }

  foreign seed_()
  foreign seed_(seed)
  foreign seed_(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16)

  foreign float()
  float(end) { float() * end }
  float(start, end) { float() * (end - start) + start }

  foreign int()
  int(end) { (float() * end).floor }
  int(start, end) { (float() * (end - start)).floor + start }

  sample(list) {
    if (list.count == 0) Fiber.abort("Not enough elements to sample.")
    return list[int(list.count)]
  }
  sample(list, count) {
    if (count > list.count) Fiber.abort("Not enough elements to sample.")

    var result = []

    // El algoritmo descrito en "Programming pearls: a sample of brilliance".
    // Usa un mapa hash para tamaños de muestra menores a 1/4 del tamaño de la
    // población y un array de booleanos para muestras más grandes. Esta
    // heurística simple mejora el rendimiento para tamaños grandes de muestra y
    // también reduce el uso de memoria.
    if (count * 4 < list.count) {
      var picked = {}
      for (i in list.count - count...list.count) {
        var index = int(i + 1)
        if (picked.containsKey(index)) index = i
        picked[index] = true
        result.add(list[index])
      }
    } else {
      var picked = List.filled(list.count, false)
      for (i in list.count - count...list.count) {
        var index = int(i + 1)
        if (picked[index]) index = i
        picked[index] = true
        result.add(list[index])
      }
    }

    return result
  }

  shuffle(list) {
    if (list.isEmpty) return

    // Barajado de Fisher-Yates.
    for (i in 0...list.count - 1) {
      var from = int(i, list.count)
      var temp = list[from]
      list[from] = list[i]
      list[i] = temp
    }
  }
}
