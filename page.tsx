"use client"

import { useState, useMemo } from "react"
import { MenuCard } from "@/components/menu-card"
import { menuItems, categories, subcategories } from "@/data/menu"
import { Button } from "@/components/ui/button"
import { Badge } from "@/components/ui/badge"
import { Input } from "@/components/ui/input"
import { ChevronDown, ChevronRight, Search, X } from "lucide-react"
import { cn } from "@/lib/utils"

export default function MenuPage() {
  const [activeCategory, setActiveCategory] = useState<string | null>(null)
  const [activeSubcategory, setActiveSubcategory] = useState<string | null>(null)
  const [expandedCategories, setExpandedCategories] = useState<string[]>(categories)
  const [searchQuery, setSearchQuery] = useState("")

  const toggleCategoryExpand = (category: string) => {
    setExpandedCategories((prev) =>
      prev.includes(category)
        ? prev.filter((c) => c !== category)
        : [...prev, category]
    )
  }

  const handleCategoryClick = (category: string | null) => {
    setActiveCategory(category)
    setActiveSubcategory(null)
  }

  const handleSubcategoryClick = (category: string, subcategory: string) => {
    setActiveCategory(category)
    setActiveSubcategory(subcategory)
    setSearchQuery("")
  }

  // Search and filter items
  const searchedItems = useMemo(() => {
    if (!searchQuery.trim()) return menuItems
    const query = searchQuery.toLowerCase().trim()
    return menuItems.filter(
      (item) =>
        item.name.toLowerCase().includes(query) ||
        item.description.toLowerCase().includes(query) ||
        item.category.toLowerCase().includes(query) ||
        (item.subcategory && item.subcategory.toLowerCase().includes(query))
    )
  }, [searchQuery])

  const filteredItems = useMemo(() => {
    const itemsToFilter = searchQuery.trim() ? searchedItems : menuItems
    return itemsToFilter.filter((item) => {
      if (!activeCategory) return true
      if (item.category !== activeCategory) return false
      if (activeSubcategory && item.subcategory !== activeSubcategory) return false
      return true
    })
  }, [searchQuery, searchedItems, activeCategory, activeSubcategory])

  const clearSearch = () => {
    setSearchQuery("")
  }

  // Group items by category and subcategory for structured display
  const getItemsByCategory = (category: string) => {
    return menuItems.filter((item) => item.category === category)
  }

  const getItemsBySubcategory = (category: string, subcategory: string) => {
    return menuItems.filter(
      (item) => item.category === category && item.subcategory === subcategory
    )
  }

  return (
    <div className="container mx-auto px-4 py-8">
      <div className="mb-8 text-center">
        <h1 className="mb-2 text-3xl font-bold text-foreground md:text-4xl">
          Nuestro Menu
        </h1>
        <p className="mb-6 text-muted-foreground">
          Selecciona los platos que deseas y agregalos al carrito
        </p>
        
        {/* Search Bar */}
        <div className="mx-auto max-w-md">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-muted-foreground" />
            <Input
              type="text"
              placeholder="Buscar platos, categorias..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="pl-10 pr-10"
            />
            {searchQuery && (
              <Button
                variant="ghost"
                size="icon"
                className="absolute right-1 top-1/2 h-7 w-7 -translate-y-1/2"
                onClick={clearSearch}
              >
                <X className="h-4 w-4" />
              </Button>
            )}
          </div>
          {searchQuery && (
            <p className="mt-2 text-sm text-muted-foreground">
              {searchedItems.length} resultado{searchedItems.length !== 1 ? "s" : ""} para &quot;{searchQuery}&quot;
            </p>
          )}
        </div>
      </div>

      <div className="flex flex-col gap-8 lg:flex-row">
        {/* Sidebar Navigation */}
        <aside className="lg:w-64 lg:shrink-0">
          <div className="lg:sticky lg:top-24">
            <nav className="space-y-1">
              <Button
                variant={activeCategory === null ? "default" : "ghost"}
                className="w-full justify-start"
                onClick={() => handleCategoryClick(null)}
              >
                Todos los Platos
                <Badge variant="secondary" className="ml-auto">
                  {menuItems.length}
                </Badge>
              </Button>

              {categories.map((category) => (
                <div key={category}>
                  <div className="flex items-center">
                    <Button
                      variant={activeCategory === category && !activeSubcategory ? "default" : "ghost"}
                      className="flex-1 justify-start"
                      onClick={() => handleCategoryClick(category)}
                    >
                      {category}
                      <Badge variant="secondary" className="ml-auto mr-2">
                        {getItemsByCategory(category).length}
                      </Badge>
                    </Button>
                    {subcategories[category] && (
                      <Button
                        variant="ghost"
                        size="icon"
                        className="h-9 w-9"
                        onClick={() => toggleCategoryExpand(category)}
                      >
                        {expandedCategories.includes(category) ? (
                          <ChevronDown className="h-4 w-4" />
                        ) : (
                          <ChevronRight className="h-4 w-4" />
                        )}
                      </Button>
                    )}
                  </div>

                  {/* Subcategories */}
                  {subcategories[category] && expandedCategories.includes(category) && (
                    <div className="ml-4 mt-1 space-y-1 border-l pl-4">
                      {subcategories[category].map((sub) => (
                        <Button
                          key={sub}
                          variant={
                            activeCategory === category && activeSubcategory === sub
                              ? "secondary"
                              : "ghost"
                          }
                          size="sm"
                          className="w-full justify-start text-sm"
                          onClick={() => handleSubcategoryClick(category, sub)}
                        >
                          {sub}
                          <Badge variant="outline" className="ml-auto text-xs">
                            {getItemsBySubcategory(category, sub).length}
                          </Badge>
                        </Button>
                      ))}
                    </div>
                  )}
                </div>
              ))}
            </nav>
          </div>
        </aside>

        {/* Main Content */}
        <main className="flex-1">
          {/* Mobile Category Pills */}
          <div className="mb-6 flex flex-wrap gap-2 lg:hidden">
            <Button
              variant={activeCategory === null ? "default" : "outline"}
              onClick={() => handleCategoryClick(null)}
              size="sm"
              className="rounded-full"
            >
              Todos
            </Button>
            {categories.map((category) => (
              <Button
                key={category}
                variant={activeCategory === category ? "default" : "outline"}
                onClick={() => handleCategoryClick(category)}
                size="sm"
                className="rounded-full"
              >
                {category}
              </Button>
            ))}
          </div>

          {/* Active Filter Display */}
          {(activeCategory || activeSubcategory) && (
            <div className="mb-6 flex items-center gap-2">
              <span className="text-sm text-muted-foreground">Mostrando:</span>
              <Badge variant="secondary">{activeCategory}</Badge>
              {activeSubcategory && (
                <>
                  <ChevronRight className="h-4 w-4 text-muted-foreground" />
                  <Badge variant="outline">{activeSubcategory}</Badge>
                </>
              )}
              <Button
                variant="ghost"
                size="sm"
                onClick={() => handleCategoryClick(null)}
                className="ml-auto text-xs"
              >
                Limpiar filtros
              </Button>
            </div>
          )}

          {/* Items Grid */}
          {activeCategory === null ? (
            // Show all items grouped by category
            <div className="space-y-12">
              {categories.map((category) => (
                <section key={category}>
                  <div className="mb-4 flex items-center gap-3">
                    <h2 className="text-2xl font-bold">{category}</h2>
                    <div className="h-px flex-1 bg-border" />
                  </div>

                  {/* Show subcategories if available */}
                  {subcategories[category] ? (
                    <div className="space-y-8">
                      {subcategories[category].map((sub) => {
                        const subItems = getItemsBySubcategory(category, sub)
                        if (subItems.length === 0) return null
                        return (
                          <div key={sub}>
                            <h3 className="mb-4 text-lg font-semibold text-muted-foreground">
                              {sub}
                            </h3>
                            <div className="grid gap-6 sm:grid-cols-2 xl:grid-cols-3">
                              {subItems.map((item) => (
                                <MenuCard key={item.id} item={item} />
                              ))}
                            </div>
                          </div>
                        )
                      })}
                    </div>
                  ) : (
                    <div className="grid gap-6 sm:grid-cols-2 xl:grid-cols-3">
                      {getItemsByCategory(category).map((item) => (
                        <MenuCard key={item.id} item={item} />
                      ))}
                    </div>
                  )}
                </section>
              ))}
            </div>
          ) : (
            // Show filtered items
            <div className="grid gap-6 sm:grid-cols-2 xl:grid-cols-3">
              {filteredItems.map((item) => (
                <MenuCard key={item.id} item={item} />
              ))}
            </div>
          )}

          {filteredItems.length === 0 && activeCategory && (
            <div className="py-12 text-center">
              <p className="text-muted-foreground">
                No hay platos disponibles en esta categoria
              </p>
            </div>
          )}
        </main>
      </div>
    </div>
  )
}
