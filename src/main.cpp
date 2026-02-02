//
//
// "Implicity kills simplicity, be loud, be clear and hold solid ground"  
//                                                            ~ Probably Me
//
//
#include <cstddef>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <array>
#include <chrono>
#include <utility>
#include <type_traits>
#include "../dependencies/Custom_ECS/include/EntityComponentSystem.hpp"
#include "../dependencies/RNG/include/random.hpp" // seed based random number generator - xorshift32
#include <SFML/Graphics.hpp>

// PLEASE READ DOCUMENATTION BEFORE FORKING OR CONTRIBUTING
// 
// NOTE : 
// 
// PHYSICAL GRID IS WHAT EXISTS IN ARRAY/MEMORY !!!
// 
// LOGICAL GRID IS JUST A LOGICAL MAPPING/(VIEW ONLY) ON TOP OF THE PHYSICAL GRID
// LOGICAL GRID DOES NOT EXISTS IN THE MEMORY, ITS A SUBSET OF THE PHYSICAL GRID // ACCESSED USING DIFFERENT METHODS
// 
// 
// LOGICAL GRID EXISTS BECAUSE THE PHYSICAL GRID COMPRISES OF PADDINGS TO IMPORVE PERFOMANCE IN HOT LOOPS AS BY MINIMIZING BRANCHING

namespace cae { // Conways's Game of Life
  // what shoud we call the cells / the live or dead entities ? 
  // we are calling it entity/entities
  
  // some static_cast shorthands
  inline std::size_t szt(auto numeric) {
    static_assert(std::is_integral<decltype(numeric)>::value, "type should be an integral");
    return static_cast<std::size_t>(numeric);
  }

  inline std::int32_t int_c(auto numeric) {
    static_assert(std::is_integral<decltype(numeric)>::value, "type should be an integral");
    return static_cast<std::int32_t>(numeric);
  }

  namespace grid_metadata {
    
    using namespace component::type;

    WidthPix CellWidth{};
    HeightPix CellHeight{};

    WidthGrid Logical_GridWidth{}; // logical
    HeightGrid Logical_GridHeight{}; // logical

    WidthGrid Physical_GridWidth{}; // physical
    HeightGrid Physical_GridHeight{}; // physical

    namespace cell_color_dead { // RGB
      std::uint8_t r = 255; // picked from coolors.io
      std::uint8_t g = 251;
      std::uint8_t b = 189;
    }

    namespace cell_color_alive {
      std::uint8_t r = 59; // picked from coolors.io
      std::uint8_t g = 193;
      std::uint8_t b = 74;
    }

    namespace current_cell_color { // state machine either 0 or 1
      std::uint8_t r[2] = { cell_color_dead::r, cell_color_alive::r };
      std::uint8_t g[2] = { cell_color_dead::g, cell_color_alive::g };
      std::uint8_t b[2] = { cell_color_dead::b, cell_color_alive::b };
    }
    
    std::int32_t padding{1}; // physical padding around the edges
    std::int32_t total_logical{}; // total logical cells/entities
    std::int32_t total_physical{};
  }

  // WORKINGS ARE ALWAYS DONE ON THE PHYSICAL GRID, as PHYSICAL GRID IS WHAT EXISTS IN ARRAY/MEMORY !!!
  // LOGICAL GRID IS JUST A LOGICAL MAPPING/(VIEW ONLY) ON TOP OF THE PHYSICAL GRID
  // LOGICAL GRID DOES NOT EXISTS IN THE MEMORY, ITS A SUBSET OF THE PHYSICAL GRID
  //
  // DEPRECATED : namespace grid_helper_p
  /* 
  namespace grid_helper_p { // _p = physical, refers to the actual physical(in memory/array) grid 
    using namespace component::type;
    std::int32_t grid_xy_to_array_index(PosGrid_x physical_x, PosGrid_y physical_y) {
      return (physical_y.get() * cae::grid_metadata::Physical_GridWidth.get()) + physical_x.get();
    }
  }
  */

  // WORKINGS ARE ALWAYS DONE ON THE PHYSICAL GRID, as PHYSICAL GRID IS WHAT EXISTS IN ARRAY/MEMORY !!!
  // LOGICAL GRID IS JUST A LOGICAL MAPPING/(VIEW ONLY) ON TOP OF THE PHYSICAL GRID
  // LOGICAL GRID DOES NOT EXISTS IN THE MEMORY, ITS A SUBSET OF THE PHYSICAL GRID
  // 
  // Input = logical | Output = physical
  // _lo = logical, refers to the entities DISPLAYED in the grid // NEVER RETURNS A LOGICAL CO-ORDINATE
  
  // DEPRECATED : namespace grid_helper_lo 
  /* 
  namespace grid_helper_lo { 
    using namespace component::type;
    
    PosGrid_x pixel_x_to_grid__x(std::int32_t x) {
      return PosGrid_x{ x / cae::grid_metadata::CellWidth.get() };
    }

    PosGrid_y pixel_y_to_grid__y(std::int32_t y) {
      return PosGrid_y{ y / cae::grid_metadata::CellHeight.get() };
    }
    

    std::size_t grid_xy_to_array_index(PosGrid_x logical_x, PosGrid_y logical_y) { // returns a physical index from logical co-ordinates
      logical_x.set(logical_x.get() + cae::grid_metadata::padding); // increment both by the padding to convert to physical cords
      logical_y.set(logical_y.get() + cae::grid_metadata::padding);
      return cae::grid_helper_p::grid_xy_to_array_index(logical_x, logical_y);
    }

    // THIS RETURNS A LOGICAL INDEX FROM LOGICAL CORDS
    std::size_t grid_xy_to_array_index_RETURN_LOGICAL(PosGrid_x logical_x, PosGrid_y logical_y) { // returns a physical index from logical co-ordinates
      return ( logical_y.get() * cae::grid_metadata::Logical_GridWidth.get() ) + logical_x.get();
    }
    
    PosGrid_x grid_xLogical_to_grid_xPhysical(PosGrid_x logical_x) {
      return PosGrid_x{ logical_x.get() + cae::grid_metadata::padding };
    }
  
    PosGrid_y grid_yLogical_to_grid_yPhysical(PosGrid_y logical_y) {
      return PosGrid_y{ logical_y.get() + cae::grid_metadata::padding };
    }

  }
  */

  namespace grid_convert {
    
    using namespace component::type;

    inline PosGrid_x Pixel_x_to_Grid_x(std::int32_t pixel_x) {
      return PosGrid_x{ pixel_x / cae::grid_metadata::CellWidth.get() };
    }

    inline PosGrid_y Pixel_y_to_Grid_y(std::int32_t pixel_y) {
      return PosGrid_y{ pixel_y / cae::grid_metadata::CellHeight.get() };
    }
    
    inline PosGrid_x Logical_x_to_Physical_x(PosGrid_x logical_x) {
      return PosGrid_x{ logical_x.get() + cae::grid_metadata::padding };
    }

    inline PosGrid_y Logical_y_to_Physical_y(PosGrid_y logical_y) {
      return PosGrid_y{ logical_y.get() + cae::grid_metadata::padding };
    }
    
    inline PosGrid_x Physical_x_to_Logical_x(PosGrid_x physical_x) {
      return PosGrid_x{ physical_x.get() - cae::grid_metadata::padding };
    }
    
    inline PosGrid_y Physical_y_to_Logical_y(PosGrid_y physical_y) {
      return PosGrid_y{ physical_y.get() - cae::grid_metadata::padding };
    }
    


    inline std::size_t Physical_xy_to_index(PosGrid_x physical_x, PosGrid_y physical_y) {
      return (szt(physical_y.get()) * szt(cae::grid_metadata::Physical_GridHeight.get())) + szt(physical_x.get());
    }

    inline std::size_t Logical_xy_to_index(PosGrid_x logical_x, PosGrid_y logical_y) {
      const PosGrid_x physical_x{ Logical_x_to_Physical_x(logical_x) };
      const PosGrid_y physical_y{ Logical_y_to_Physical_y(logical_y) };
      return (szt(physical_y.get()) * szt(cae::grid_metadata::Physical_GridHeight.get())) + szt(physical_x.get());
    }
    
    // for when i need the logical index for some reason
    inline std::size_t Logical_xy_to_index_RETURN_LOGICAL(PosGrid_x logical_x, PosGrid_y logical_y) {
      return (szt(logical_y.get()) * szt(cae::grid_metadata::Logical_GridHeight.get())) + szt(logical_x.get());
    }

  }

  struct Renderables {
    static inline sf::VertexArray entities_VertexArray;
    static inline sf::VertexArray border_vertical;
    static inline sf::VertexArray border_horizontal;
    static inline sf::Color entities_Color;
  };
  
  namespace grid_iterator::for_each {
  
    template <typename Fn>
    void physical_cell(Fn&& task) { //
      using namespace component::type;

      const WidthGrid physical_width{ cae::grid_metadata::Physical_GridWidth };
      const HeightGrid physical_height{ cae::grid_metadata::Physical_GridHeight };      

      for (PosGrid_y physical_y{ 0 }; physical_y.get() < physical_height.get(); physical_y.set(physical_y.get() + 1)) {
        for (PosGrid_x physical_x{ 0 }; physical_x.get() < physical_width.get(); physical_x.set(physical_x.get() + 1)) {
          
          const std::size_t index = cae::grid_convert::Physical_xy_to_index(physical_x, physical_y);
          task(physical_x, physical_y, index); // perfectly forwaded lamda

        }
      }

    }

    template <typename Fn>
    void logical_cell(Fn&& task) {
      using namespace component::type;

      const WidthGrid logical_width{ cae::grid_metadata::Logical_GridWidth };
      const HeightGrid logical_height{ cae::grid_metadata::Logical_GridHeight };


      for (PosGrid_y logical_y{ 0 }; logical_y.get() < logical_height.get(); logical_y.set(logical_y.get() + 1)) {
        for (PosGrid_x logical_x{ 0 }; logical_x.get() < logical_width.get(); logical_x.set(logical_x.get() + 1)) {
              
          const std::size_t index = cae::grid_convert::Logical_xy_to_index(logical_x, logical_y);
          task(logical_x, logical_y, index);
        
        }
      }
    }

    template <typename Fn>
    void ranged_physical_cell(const component::type::PosGrid_x physical_x,
                              const component::type::PosGrid_y physical_y,
                              const component::type::WidthGrid range_width,
                              const component::type::HeightGrid range_height,
                              Fn&& task) { //

      using namespace component::type;

      const PosGrid_x start_x{ physical_x };
      const PosGrid_y start_y{ physical_y };

      const PosGrid_x end_x{ physical_x.get() + range_width.get() };
      const PosGrid_y end_y{ physical_y.get() + range_height.get() };

      for (PosGrid_y current_y{ start_y }; current_y.get() < end_y.get(); current_y.set(current_y.get() + 1)) {
        for (PosGrid_x current_x{ start_x }; current_x.get() < end_x.get(); current_x.set(current_x.get() + 1)) {

          const std::size_t index = cae::grid_convert::Physical_xy_to_index(current_x, current_y);
          task(current_x, current_y, index); // perfectly forwaded lamda

        }
      }

    }

    
    // the cells which are used for padding
    template <typename Fn>
    void physical_padded_cell(Fn&& task_lambda_ARGS_x_y_index) {
      using namespace component::type;
      
      PosGrid_x start_x{};
      PosGrid_y start_y{};
      const PosGrid_x max_x{ cae::grid_metadata::Physical_GridWidth.get() - 1 };
      const PosGrid_y max_y{ cae::grid_metadata::Physical_GridHeight.get() - 1 };

      WidthGrid range_width{};
      HeightGrid range_height{};

      const std::int32_t& padd = cae::grid_metadata::padding;

      // top left to top right (inclusive)
      start_x.set(0);
      start_y.set(0);
      range_width.set(cae::grid_metadata::Physical_GridWidth.get());
      range_height.set(padd);

      for_each::ranged_physical_cell(start_x, start_y, range_width, range_height,
        task_lambda_ARGS_x_y_index
      );

    
      // bottom left to bottom right (inclusive)
      start_x.set(0);
      start_y.set(cae::grid_metadata::Physical_GridHeight.get() - padd); // bottom
      range_width.set(cae::grid_metadata::Physical_GridWidth.get());
      range_height.set(padd);

      for_each::ranged_physical_cell(start_x, start_y, range_width, range_height,
        task_lambda_ARGS_x_y_index
      );


      // top left to bottom left (EXCLUSIVE) or LOGICAL
      start_x.set(0);
      start_y.set(padd); 
      range_width.set(padd);
      range_height.set(cae::grid_metadata::Logical_GridHeight.get()); // as padding is on both top and bottom

      for_each::ranged_physical_cell(start_x, start_y, range_width, range_height,
        task_lambda_ARGS_x_y_index
      );

      // top right to bottom right (EXCLUSIVE) or LOGICAL
      start_x.set(cae::grid_metadata::Physical_GridWidth.get() - padd);
      start_y.set(padd);
      range_width.set(padd);
      range_height.set(cae::grid_metadata::Logical_GridHeight.get()); // as padding is on both top and bottom

      for_each::ranged_physical_cell(start_x, start_y, range_width, range_height,
        task_lambda_ARGS_x_y_index
      );

    }

  }
  
  // user api
  void init_grid(std::int32_t grid_width, std::int32_t grid_height, std::int32_t cell_width, std::int32_t cell_height, 
                 std::int32_t grid_padding = 1) { // the dimentions you want
    using namespace cae::grid_metadata;

    padding = grid_padding;

    cae::grid_metadata::Physical_GridWidth.set(grid_width + (padding * 2)); // padding on every side
    cae::grid_metadata::Physical_GridHeight.set(grid_height + (padding * 2));

    cae::grid_metadata::Logical_GridWidth.set(grid_width);
    cae::grid_metadata::Logical_GridHeight.set(grid_height);

    cae::grid_metadata::total_logical = grid_width* grid_height;

    cae::grid_metadata::total_physical = cae::grid_metadata::Physical_GridWidth.get() * cae::grid_metadata::Physical_GridHeight.get();

    cae::grid_metadata::CellWidth.set(cell_width);
    cae::grid_metadata::CellHeight.set(cell_height);

  }

  template <typename key, typename link>
  void create_entities(myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace cae::grid_iterator;

    for_each::physical_cell(
      [&](auto x, auto y, std::size_t index) {
        cell_index_to_entity.insert(index, myecs::create_entity());
      }
    );
    
  }
  
  template <typename key, typename link>
  void init_entities(const myecs::sparse_set<key, link>& cell_index_to_entity, const component::type::WidthPix cell_width, const component::type::HeightPix cell_height, std::uint32_t initial_seed = 0) {
    using namespace cae::grid_iterator;

    // ECS registry/storage initialization
    for_each::logical_cell(
      [&](auto x, auto y, std::size_t index) {
        myecs::add_comp_to<comp::position>(cell_index_to_entity.at(index));
        myecs::add_comp_to<comp::position_grid>(cell_index_to_entity.at(index));
        myecs::add_comp_to<comp::rectangle>(cell_index_to_entity.at(index));
        myecs::add_comp_to<comp::alive>(cell_index_to_entity.at(index));
        myecs::add_comp_to<comp::neighbour>(cell_index_to_entity.at(index));
      }
    );
    
    for_each::physical_padded_cell(
      [&](auto x, auto y, std::size_t index) {
        myecs::add_comp_to<comp::position_grid>(cell_index_to_entity.at(index));
        myecs::add_comp_to<comp::alive>(cell_index_to_entity.at(index));
      }
    );



    // default values initialization
    for_each::logical_cell(
      [&](auto x, auto y, std::size_t index) {
        ecs_access(comp::alive, cell_index_to_entity.at(index), value).set(
          static_cast<bool>(mgl::xorshift32(initial_seed) % 2) // either 1 or 0 // random dead or alive
        );
        ecs_access(comp::rectangle, cell_index_to_entity.at(index), width).set(cell_width.get());
        ecs_access(comp::rectangle, cell_index_to_entity.at(index), height).set(cell_height.get());
      }
    );

    for_each::physical_padded_cell(
      [&](auto x, auto y, std::size_t index) {
        ecs_access(comp::alive, cell_index_to_entity.at(index), value).set(false); // permanently dead
      }
    );
    
  }

  template <typename key, typename link>
  void init_entities_pos(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    
    assert( // invariant check, just in case
      cae::grid_metadata::Physical_GridWidth.get() * cae::grid_metadata::Physical_GridHeight.get() == cell_index_to_entity.dense.size()
      && "Entity array not equal to width * height"
    );
    
    using namespace cae::grid_iterator;
    
    for_each::logical_cell(
      [&](auto x, auto y, std::size_t index) {

        const std::int32_t x_pix = x.get() * grid_metadata::CellWidth.get();
        const std::int32_t y_pix = y.get() * grid_metadata::CellHeight.get();;

        ecs_access(comp::position, cell_index_to_entity.at(index), x).set(x_pix);
        ecs_access(comp::position, cell_index_to_entity.at(index), y).set(y_pix);

        ecs_access(comp::position_grid, cell_index_to_entity.at(index), x).set(x.get());
        ecs_access(comp::position_grid, cell_index_to_entity.at(index), y).set(y.get());
      }
    );

  }

  template <typename key, typename link>
  void update_entities_VertexArray(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace cae::grid_iterator;
   
    sf::Color current_entity_color;
   
    for_each::logical_cell(
      [&](auto logical_x, auto logical_y, std::size_t index) {
        
        // logical index for accessing vertex arrays
        const std::size_t logical_i = cae::grid_convert::Logical_xy_to_index_RETURN_LOGICAL(logical_x, logical_y);
        
        const std::int32_t x_pix = ecs_access(comp::position, cell_index_to_entity.at(index), x).get();
        const std::int32_t y_pix = ecs_access(comp::position, cell_index_to_entity.at(index), y).get();
        const std::int32_t width = ecs_access(comp::rectangle, cell_index_to_entity.at(index), width).get();
        const std::int32_t height = ecs_access(comp::rectangle, cell_index_to_entity.at(index), height).get();

        const auto current_alive = ecs_access(comp::alive, cell_index_to_entity.at(index), value).get();

        current_entity_color.r = cae::grid_metadata::current_cell_color::r[current_alive];
        current_entity_color.g = cae::grid_metadata::current_cell_color::g[current_alive];
        current_entity_color.b = cae::grid_metadata::current_cell_color::b[current_alive];

        const std::size_t base = logical_i * 4;

        Renderables::entities_VertexArray[base + 0].position.x = x_pix;          // top left
        Renderables::entities_VertexArray[base + 0].position.y = y_pix;

        Renderables::entities_VertexArray[base + 1].position.x = x_pix + width;  // top right
        Renderables::entities_VertexArray[base + 1].position.y = y_pix;

        Renderables::entities_VertexArray[base + 2].position.x = x_pix + width;  // bottom right
        Renderables::entities_VertexArray[base + 2].position.y = y_pix + height;

        Renderables::entities_VertexArray[base + 3].position.x = x_pix;
        Renderables::entities_VertexArray[base + 3].position.y = y_pix + height; // bottom left

        Renderables::entities_VertexArray[base + 0].color = current_entity_color;
        Renderables::entities_VertexArray[base + 1].color = current_entity_color;
        Renderables::entities_VertexArray[base + 2].color = current_entity_color;
        Renderables::entities_VertexArray[base + 3].color = current_entity_color;
      }
    );
    
  }

  template <typename key, typename link>
  void init_entities_VertexArray(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    Renderables::entities_VertexArray.setPrimitiveType(sf::Quads);
    Renderables::entities_VertexArray.resize(cae::grid_metadata::total_logical * 4);
    

    update_entities_VertexArray(cell_index_to_entity);
  }


  template <typename key, typename link>
  void update_entities_VertexArray_state_only(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace cae::grid_iterator;

    sf::Color current_entity_color;

    for_each::logical_cell(
      [&](auto logical_x, auto logical_y, std::size_t index) {

        const std::size_t logical_i = cae::grid_convert::Logical_xy_to_index_RETURN_LOGICAL(logical_x, logical_y);

        const auto current_alive = ecs_access(comp::alive, cell_index_to_entity.at(index), value).get();

        current_entity_color.r = cae::grid_metadata::current_cell_color::r[current_alive];
        current_entity_color.g = cae::grid_metadata::current_cell_color::g[current_alive];
        current_entity_color.b = cae::grid_metadata::current_cell_color::b[current_alive];;

        const std::size_t base = logical_i * 4;

        Renderables::entities_VertexArray[base + 0].color = current_entity_color;
        Renderables::entities_VertexArray[base + 1].color = current_entity_color;
        Renderables::entities_VertexArray[base + 2].color = current_entity_color;
        Renderables::entities_VertexArray[base + 3].color = current_entity_color;

      }
    );

  }


  void update_border_VertexArray() { // lo = logical
    // OPERATING ON THE VISUAL PART BY REFERRING TO LOGICAL GRID
    
    std::int32_t total_h = grid_metadata::Logical_GridHeight.get() + 1; // total horizontal lines, includes overlapped
    std::int32_t total_v = grid_metadata::Logical_GridWidth.get() + 1; // total vertical lines, includes overlapped

    std::int32_t max_x = (grid_metadata::Logical_GridWidth.get() * grid_metadata::CellWidth.get()) - 1;
    std::int32_t max_y = (grid_metadata::Logical_GridHeight.get() * grid_metadata::CellHeight.get()) - 1;
    
    std::int32_t width = 0; 
    std::int32_t height = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::size_t base = 0;
    
    // for horizontal lines -------------------------

    width = max_x + 1; // for horizontal lines
    height = 1;// for horizontal lines
    x = 0; // as they are horizontal lines, all starting x pos will be zero (0)
    for (std::int32_t i = 0; i < total_h; ++i) {
      
      base = i * 4;
     
      y = i * grid_metadata::CellHeight.get();

      Renderables::border_horizontal[base + 0].position.x = x; // top left
      Renderables::border_horizontal[base + 0].position.y = y;

      Renderables::border_horizontal[base + 1].position.x = x + width; // top right
      Renderables::border_horizontal[base + 1].position.y = y;

      Renderables::border_horizontal[base + 2].position.x = x + width; // bottom right
      Renderables::border_horizontal[base + 2].position.y = y + height;
      
      Renderables::border_horizontal[base + 3].position.x = x;        // bottom left
      Renderables::border_horizontal[base + 3].position.y = y + height;

      Renderables::border_horizontal[base + 0].color = sf::Color::Cyan;
      Renderables::border_horizontal[base + 1].color = sf::Color::Cyan;
      Renderables::border_horizontal[base + 2].color = sf::Color::Cyan;
      Renderables::border_horizontal[base + 3].color = sf::Color::Cyan;
    }

    
    // dealing with the last quad
    base = (total_h - 1) * 4; // last quad index
    y = max_y; // we are drawing the last line at the last y index

    Renderables::border_horizontal[base + 0].position.x = x; // top left
    Renderables::border_horizontal[base + 0].position.y = y;

    Renderables::border_horizontal[base + 1].position.x = x + width; // top right
    Renderables::border_horizontal[base + 1].position.y = y;

    Renderables::border_horizontal[base + 2].position.x = x + width; // bottom right
    Renderables::border_horizontal[base + 2].position.y = y + height;

    Renderables::border_horizontal[base + 3].position.x = x;        // bottom left
    Renderables::border_horizontal[base + 3].position.y = y + height;

    


    // for vertical lines ------------------------
    width = 1;
    height = max_y + 1;
    y = 0; // as we are now dealing with vertical lines all starting y pos will be 0

    for (std::int32_t i = 0; i < total_v; ++i) { // vertical lines

      base = i * 4;
      x = i * grid_metadata::CellWidth.get();

      Renderables::border_vertical[base + 0].position.x = x; //  top left
      Renderables::border_vertical[base + 0].position.y = y;

      Renderables::border_vertical[base + 1].position.x = x + width; // top right
      Renderables::border_vertical[base + 1].position.y = y; // top right

      Renderables::border_vertical[base + 2].position.x = x + width; // bottom right
      Renderables::border_vertical[base + 2].position.y = y + height;

      Renderables::border_vertical[base + 3].position.x = x; // bottom left 
      Renderables::border_vertical[base + 3].position.y = y + height;

      Renderables::border_vertical[base + 0].color = sf::Color::Cyan;
      Renderables::border_vertical[base + 1].color = sf::Color::Cyan;
      Renderables::border_vertical[base + 2].color = sf::Color::Cyan;
      Renderables::border_vertical[base + 3].color = sf::Color::Cyan;
    }

    base = (total_v - 1) * 4; // last quad index
    x = max_x;

    Renderables::border_vertical[base + 0].position.x = x; //  top left
    Renderables::border_vertical[base + 0].position.y = y;

    Renderables::border_vertical[base + 1].position.x = x + width; // top right
    Renderables::border_vertical[base + 1].position.y = y; // top right

    Renderables::border_vertical[base + 2].position.x = x + width; // bottom right
    Renderables::border_vertical[base + 2].position.y = y + height;

    Renderables::border_vertical[base + 3].position.x = x; // bottom left 
    Renderables::border_vertical[base + 3].position.y = y + height;

  }

  void init_border_VertexArray() {
    Renderables::border_vertical.setPrimitiveType(sf::Quads);
    Renderables::border_horizontal.setPrimitiveType(sf::Quads);
    
    Renderables::border_horizontal.resize(
      (grid_metadata::Logical_GridHeight.get() + 1) * 4
    );

    Renderables::border_vertical.resize(
      (grid_metadata::Logical_GridWidth.get() + 1) * 4
    );

    update_border_VertexArray();
  }


  template <typename key, typename link>
  std::int32_t nth_cell_alive_neighbours(const myecs::sparse_set<key, link>& cell_index_to_entity,
    const component::type::PosGrid_x logical_x,
    const component::type::PosGrid_y logical_y) { // takes logical index
    using namespace component::type;

    const std::size_t center_cell_index = cae::grid_convert::Logical_xy_to_index(logical_x, logical_y);

    const std::int32_t TopmostLeft_offset = cae::grid_metadata::padding;

    const PosGrid_x physical_neighbour_TopmostLeft_x{ grid_convert::Logical_x_to_Physical_x(logical_x).get() - TopmostLeft_offset };
    const PosGrid_y physical_neighbour_TopmostLeft_y{ grid_convert::Logical_y_to_Physical_y(logical_y).get() - TopmostLeft_offset };

    const PosGrid_x start_x{ physical_neighbour_TopmostLeft_x };
    const PosGrid_y start_y{ physical_neighbour_TopmostLeft_y };

    const WidthGrid range_width{ (cae::grid_metadata::padding * 2) + 1 }; // *2 because padding is in both sides and + 1 to also include the center (main) cell
    const HeightGrid range_height{ (cae::grid_metadata::padding * 2) + 1 };

    std::int32_t total_neighbours{ 0 };

    cae::grid_iterator::
    for_each::ranged_physical_cell( 
      start_x,
      start_y,
      range_width,
      range_height,
      [&](auto x, auto y, std::size_t index) {
        total_neighbours += ecs_access(comp::alive, cell_index_to_entity.at(index), value).get();
      }
    );


    total_neighbours -= ecs_access(comp::alive, cell_index_to_entity.at(center_cell_index), value).get();
    return total_neighbours;
  }


  template <typename key, typename link>
  void calculate_alive_neighbours(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace cae::grid_iterator;

    for_each::logical_cell(
      [&](auto logical_x, auto logical_y, std::size_t index) {
        ecs_access(comp::neighbour, cell_index_to_entity.at(index), count) =
          nth_cell_alive_neighbours(
            cell_index_to_entity,
            logical_x,
            logical_y
          );
      }
    );  
   
  }

  template <typename key, typename link>
  void print_everycell_neighbour_count(const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace cae::grid_iterator;
    
    for_each::logical_cell(cell_index_to_entity,

      [&](auto x, auto y, std::size_t index) {
        
        const std::size_t logical_index = cae::grid_convert::Logical_xy_to_index_RETURN_LOGICAL(x, y);
        
        std::cout << "cell " << logical_index << " total neighbours : "
          << ecs_access(comp::neighbour, cell_index_to_entity.at(index), count)
          << std::endl;
      }

    );
  }

  

  namespace rulebook {
    /* source: https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
      1.Any live cell with fewer than two live neighbours dies, as if by underpopulation.
      
      2.Any live cell with two or three live neighbours lives on to the next generation.
      
      3.Any live cell with more than three live neighbours dies, as if by overpopulation.
      
      4.Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    */

    

    // rules implemented below
  }
}

namespace cae::rulebook {
  void apply_rules(const entity current_id) {
    const auto& neighbours = ecs_access(comp::neighbour, current_id, count);
    const bool current_alive = ecs_access(comp::alive, current_id, value).get();
    
    const bool next_alive = (neighbours == 3) || (current_alive && neighbours == 2);

    ecs_access(comp::alive, current_id, value).set(next_alive);
  }
}

namespace cae::input {
  bool is_paused() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) return true;
    return false;
  }

  bool is_drawing() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
      return true;
    }
    else {
      return false;
    }
  }


  bool is_erasing() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
      return true;
    }
    else {
      return false;
    }
  }

  template <typename key, typename link>
  void draw(sf::RenderWindow& window, const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace component::type;
    sf::Vector2i mousepos = sf::Mouse::getPosition(window);

    PosPix_x mouse_x{ mousepos.x };
    PosPix_y mouse_y{ mousepos.y };

    // view cordinates // is never physical
    PosGrid_x mouse_grid_x{ cae::grid_convert::Pixel_x_to_Grid_x(mouse_x.get()) };
    PosGrid_y mouse_grid_y{ cae::grid_convert::Pixel_y_to_Grid_y(mouse_y.get()) };

    std::size_t index = cae::grid_convert::Logical_xy_to_index(mouse_grid_x, mouse_grid_y);
    /*
    if (!window.hasFocus()) {
      std::cout << "\nCancelled Draw, Window out of focus\n";
      return;
    }*/
    ecs_access(comp::alive, cell_index_to_entity.at(index), value).set(true);
    
  }

  template <typename key, typename link>
  void erase(sf::RenderWindow& window, const myecs::sparse_set<key, link>& cell_index_to_entity) {
    using namespace component::type;
    sf::Vector2i mousepos = sf::Mouse::getPosition(window);

    PosPix_x mouse_x{ mousepos.x };
    PosPix_y mouse_y{ mousepos.y };

    // view cordinates // is never physical
    PosGrid_x mouse_grid_x{ cae::grid_convert::Pixel_x_to_Grid_x(mouse_x.get()) };
    PosGrid_y mouse_grid_y{ cae::grid_convert::Pixel_y_to_Grid_y(mouse_y.get()) };

    std::size_t index = cae::grid_convert::Logical_xy_to_index(mouse_grid_x, mouse_grid_y);
    /*
    if (!window.hasFocus()) {
      std::cout << "\nCancelled Erase, Window out of focus\n";
      return;
    }*/
    ecs_access(comp::alive, cell_index_to_entity.at(index), value).set(false);

  }

}

template <typename key, typename link>
void conways_game_of_life(const myecs::sparse_set<key, link>& cell_index_to_entity) {
  using namespace cae::grid_iterator;

  for_each::logical_cell(
    [&](auto x, auto y, std::size_t index) {
      cae::rulebook::apply_rules(cell_index_to_entity.at(index));
    }
  );

}

namespace Profile {
  constexpr std::size_t total_ticks_to_profile = 10;
  bool ended = false;
  
  using clock = std::chrono::steady_clock;
  using ns = std::chrono::nanoseconds;

  struct Timer {
    std::string ProfileName;
    std::array<std::uint64_t, total_ticks_to_profile> ProfileTimeRingBuffer;
    clock::time_point ProfileStartTime;
    clock::time_point ProfileEndTime;
    std::uint64_t CurrentWriteIndex;

    explicit Timer(const std::string& ProfileName) 
      : ProfileName(ProfileName), 
        CurrentWriteIndex(0)
    {
    }
    
    void WriteToBuffer(auto elasped) {
      if (CurrentWriteIndex >= total_ticks_to_profile) CurrentWriteIndex = 0;
      ProfileTimeRingBuffer[CurrentWriteIndex] = elasped;
      ++CurrentWriteIndex;
    }

    void StartProfile() {
      ProfileStartTime = clock::now();
    }
    
    void EndProfile() {
      ProfileEndTime = clock::now();
      auto elasped = std::chrono::duration_cast<ns>(ProfileEndTime - ProfileStartTime).count();
      WriteToBuffer(elasped);
    }

    template <typename Fn>
    void Profile_it(Fn&& Method_lambda) {
      StartProfile();
      Method_lambda();
      EndProfile();
    }

    void dump_buffer() {
      std::cout << "Dumping Profile Buffer : " << ProfileName << '\n';
      for (std::size_t i = 0; i < total_ticks_to_profile; ++i) {
        double elasped_ms = std::chrono::duration<double, std::milli>(std::chrono::nanoseconds(ProfileTimeRingBuffer[i])).count();
        std::cout << "Buffer " << i << " th = " << elasped_ms << " ms" << '\n';
      }
    }
  };



}

int main() {

  Profile::Timer Profile1("Calculating Alive Neighbours");
  Profile::Timer Profile2("Applying conways game of life, rules");

  //std::uint32_t CAE_SEED = mgl::make_seed_xorshift32(); // mgl = my game library
  std::uint32_t CAE_SEED = 0;
  // 
  // Some seeds i found:
  //
  // Beautiful Propagator : 210340070
  // Weird oscilator : 2797493222
  //
  std::cout << "Current seed : " << CAE_SEED << std::endl;

  cae::init_grid(40, 40, 20, 20);  

  const component::type::WidthPix DisplayWindow_Width{ cae::grid_metadata::CellWidth.get() * cae::grid_metadata::Logical_GridWidth.get()};
  const component::type::HeightPix DisplayWindow_Height{ cae::grid_metadata::CellHeight.get() * cae::grid_metadata::Logical_GridHeight.get() };

  sf::RenderWindow DisplayWindow(sf::VideoMode(DisplayWindow_Width.get(), DisplayWindow_Height.get()), "Cellular Automata Engine (Runnig: Comway's Game of Life) | Hold LCtrl to pause | Left click to draw, Right click to erase");
  sf::Event event;
  //DisplayWindow.setFramerateLimit(8);

  
  myecs::sparse_set<std::uint32_t, entity> cell_index_to_entity; // REFERRES TO PHYSICAL //  will have padding of one cell around the edges
  myecs::sparse_set<std::uint32_t, entity> NextGen_buffer; // will only hold dead or alive, only for the next generation / tick
 
  cae::create_entities(cell_index_to_entity); // creates the total number of entities
  

  cae::init_entities(
    cell_index_to_entity,
    cae::grid_metadata::CellWidth, 
    cae::grid_metadata::CellHeight,
    CAE_SEED // <-- this creates noise (random dead or alive)
  ); // adds necessarry components to the entities
  

  cae::init_entities_pos(cell_index_to_entity);
  
  cae::init_entities_VertexArray(cell_index_to_entity);

  cae::init_border_VertexArray();
  
  //cae::calculate_alive_neighbours(cell_index_to_entity);
  //cae::print_everycell_neighbour_count(cell_index_to_entity);

  while (DisplayWindow.isOpen()) {
    
    while (DisplayWindow.pollEvent(event)) {
      if (event.type == sf::Event::Closed) DisplayWindow.close();
    }

    if (cae::input::is_drawing() && DisplayWindow.hasFocus()) {
      cae::input::draw(DisplayWindow, cell_index_to_entity);
    }
    else if (cae::input::is_erasing() && DisplayWindow.hasFocus()) {
      cae::input::erase(DisplayWindow, cell_index_to_entity);
    }
    else if (!cae::input::is_paused() && DisplayWindow.hasFocus()){
      Profile1.Profile_it(
        [&]() {
          cae::calculate_alive_neighbours(cell_index_to_entity);
        }
      );

      Profile2.Profile_it(
        [&]() {
          conways_game_of_life(cell_index_to_entity);
        }
      );

    }

    cae::update_entities_VertexArray_state_only(cell_index_to_entity);
    DisplayWindow.clear(sf::Color::Black);
    DisplayWindow.draw(cae::Renderables::entities_VertexArray);
    DisplayWindow.draw(cae::Renderables::border_horizontal);
    DisplayWindow.draw(cae::Renderables::border_vertical);
    
    DisplayWindow.display();
  }

  Profile1.dump_buffer();
  Profile2.dump_buffer();

  return 0;
}