/**
 *	A List Box Implementation
 *	Nana C++ Library(http://www.nanapro.org)
 *	Copyright(C) 2003-2016 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Boost Software License, Version 1.0. 
 *	(See accompanying file LICENSE_1_0.txt or copy at 
 *	http://www.boost.org/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/widgets/listbox.hpp
 *	@contributors:
 *		Hiroshi Seki
 *		Ariel Vina-Rodriguez
 *		leobackes(pr#86,pr#97)
 *		Benjamin Navarro(pr#81)
 *		besh81(pr#130)
 */

#ifndef NANA_GUI_WIDGETS_LISTBOX_HPP
#define NANA_GUI_WIDGETS_LISTBOX_HPP
#include <nana/push_ignore_diagnostic>

#include "widget.hpp"
#include "detail/inline_widget.hpp"
#include <nana/pat/abstract_factory.hpp>
#include <nana/concepts.hpp>
#include <nana/key_type.hpp>
#include <functional>
#include <initializer_list>

namespace nana
{
	class listbox;

	namespace drawerbase
	{
		namespace listbox
		{
			using size_type = std::size_t;
			using native_string_type = ::nana::detail::native_string_type;

			/// usefull for both absolute and display (sorted) positions
			struct index_pair
			{
				size_type cat;	//The pos of category
				size_type item;	//the pos of item in a category.

				index_pair(size_type cat_pos = 0, size_type item_pos = 0)
					: cat(cat_pos),
					item(item_pos)
				{}

				bool empty() const
				{
					return (npos == cat);
				}

				void set_both(size_type n)
				{
					cat = item = n;
				}

				bool is_category() const
				{
					return (npos != cat && npos == item);
				}

				bool is_item() const
				{
					return (npos != cat && npos != item);
				}

				bool operator==(const index_pair& r) const
				{
					return (r.cat == cat && r.item == item);
				}

				bool operator!=(const index_pair& r) const
				{
					return !this->operator==(r);
				}

				bool operator>(const index_pair& r) const
				{
					return (cat > r.cat) || (cat == r.cat && item > r.item);
				}
			};

			using selection = std::vector<index_pair>;

			using inline_notifier_interface = detail::inline_widget_notifier_interface<index_pair, std::string>;

			struct cell
			{
				struct format
				{
					::nana::color bgcolor;
					::nana::color fgcolor;
                    /// ::nana::paint::font font;  \todo 
					format() = default;
					format(const ::nana::color& bgcolor, const ::nana::color& fgcolor);
				};

				using format_ptr = ::std::unique_ptr<format>;

				::std::string	text;
				format_ptr	custom_format;

				cell() = default;
				cell(const cell&);
				cell(cell&&);
				cell(::std::string);
				cell(::std::string, const format&);
				cell(::std::string, const ::nana::color& bgcolor, const ::nana::color& fgcolor);

				cell& operator=(const cell&);
				cell& operator=(cell&&);
			};

			class oresolver
			{
			public:
				oresolver& operator<<(bool);
				oresolver& operator<<(short);
				oresolver& operator<<(unsigned short);
				oresolver& operator<<(int);
				oresolver& operator<<(unsigned int);
				oresolver& operator<<(long);
				oresolver& operator<<(unsigned long);
				oresolver& operator<<(long long);
				oresolver& operator<<(unsigned long long);
				oresolver& operator<<(float);
				oresolver& operator<<(double);
				oresolver& operator<<(long double);

				oresolver& operator<<(const char* text_utf8);
				oresolver& operator<<(const wchar_t*);
				oresolver& operator<<(const std::string& text_utf8);
				oresolver& operator<<(const std::wstring&);
				oresolver& operator<<(std::wstring&&);
				oresolver& operator<<(cell);
				oresolver& operator<<(std::nullptr_t);

				std::vector<cell> && move_cells();
			private:
				std::vector<cell> cells_;
			};

			class iresolver
			{
			public:
				iresolver(const std::vector<cell>&);

				iresolver& operator>>(bool&);
				iresolver& operator>>(short&);
				iresolver& operator>>(unsigned short&);
				iresolver& operator>>(int&);
				iresolver& operator>>(unsigned int&);
				iresolver& operator>>(long&);
				iresolver& operator>>(unsigned long&);
				iresolver& operator>>(long long&);
				iresolver& operator>>(unsigned long long&);
				iresolver& operator>>(float&);
				iresolver& operator>>(double&);
				iresolver& operator>>(long double&);

				iresolver& operator>>(std::string& text_utf8);
				iresolver& operator>>(std::wstring&);
				iresolver& operator>>(cell&);
				iresolver& operator>>(std::nullptr_t);
			private:
				const std::vector<cell>& cells_;
				std::size_t pos_{0};
			};

			using selection = std::vector<index_pair>;

			/// struct essence_t
			///@brief:	this struct gives many data for listbox,
			///			the state of the struct does not effect on member funcions, therefore all data members are public.
			struct essence_t;

			struct category_t;
			class drawer_header_impl;
			class drawer_lister_impl;

			/// mostly works on display positions
			class trigger: public drawer_trigger
			{
			public:
				trigger();
				~trigger();
				essence_t& essence() const;
			private:
				void _m_draw_border();
			private:
				void attached(widget_reference, graph_reference)	override;
				void detached()	override;
				void typeface_changed(graph_reference)	override;
				void refresh(graph_reference)	override;
				void mouse_move(graph_reference, const arg_mouse&)	override;
				void mouse_leave(graph_reference, const arg_mouse&)	override;
				void mouse_down(graph_reference, const arg_mouse&)	override;
				void mouse_up(graph_reference, const arg_mouse&)	override;
				void mouse_wheel(graph_reference, const arg_wheel&)	override;
				void dbl_click(graph_reference, const arg_mouse&)	override;
				void resized(graph_reference, const arg_resized&)		override;
				void key_press(graph_reference, const arg_keyboard&)	override;
				void key_char(graph_reference, const arg_keyboard&)	override;
			private:
				essence_t * essence_;
				drawer_header_impl *drawer_header_;
				drawer_lister_impl *drawer_lister_;
			};//end class trigger

			/// operate with absolute positions and contain only the position but montain pointers to parts of the real items 
			/// item_proxy self, it references and iterators are not invalidated by sort()
			class item_proxy
				: public std::iterator<std::input_iterator_tag, item_proxy>
			{
			public:
				item_proxy(essence_t*);
				item_proxy(essence_t*, const index_pair&);

                /// the main porpose of this it to make obvious that item_proxy operate with absolute positions, and dont get moved during sort()
                static item_proxy from_display(essence_t *ess, const index_pair &relative) ;
                item_proxy from_display(const index_pair &relative) const;

                /// posible use: last_selected_display = last_selected.to_display().item; use with caution, it get invalidated after a sort()
                index_pair to_display() const;

				bool empty() const;

				item_proxy & check(bool ck);
				bool checked() const;

				item_proxy & select(bool);
				bool selected() const;

				item_proxy & bgcolor(const nana::color&);
				nana::color bgcolor() const;

				item_proxy& fgcolor(const nana::color&);
				nana::color fgcolor() const;

				index_pair pos() const;

				size_type columns() const;

				item_proxy&		text(size_type col, cell);
				item_proxy&		text(size_type col, std::string);
				item_proxy&		text(size_type col, std::wstring);
				std::string	text(size_type col) const;

				void icon(const nana::paint::image&);

				template<typename T>
				item_proxy & resolve_from(const T& t)
				{
					oresolver ores;
					ores << t;
					auto && cells = ores.move_cells();
					auto cols = columns();
					cells.resize(cols);
					for (auto pos = 0u; pos < cols; ++pos)
					{
						auto & el = cells[pos];
						if (el.text.size() == 1 && el.text[0] == '\0')
							continue;
						text(pos, std::move(el));
					}
					
					return *this;
				}

				template<typename T>
				void resolve_to(T& t) const
				{
					iresolver ires(_m_cells());
					ires >> t;
				}

				template<typename T>
				T* value_ptr() const
				{
					return any_cast<T>(_m_value());
				}

				template<typename T>
				T & value() const
				{
					auto * pany = _m_value();
					if(nullptr == pany)
						throw std::runtime_error("listbox::item_proxy.value<T>() is empty");

					T * p = any_cast<T>(_m_value());
					if(nullptr == p)
						throw std::runtime_error("listbox::item_proxy.value<T>() invalid type of value");
					return *p;
				}
				template<typename T>
				T & value() 
				{
					auto * pany = _m_value();
					if (nullptr == pany)
						throw std::runtime_error("listbox::item_proxy.value<T>() is empty");

					T * p = any_cast<T>(_m_value(false));
					if (nullptr == p)
						throw std::runtime_error("listbox::item_proxy.value<T>() invalid type of value");
					return *p;
				}
				template<typename T>
				item_proxy & value(T&& t)
				{
					*_m_value(true) = std::forward<T>(t);
					return *this;
				}

				/// Behavior of Iterator's value_type
				bool operator==(const char * s) const;
				bool operator==(const wchar_t * s) const;
				bool operator==(const ::std::string& s) const;
				bool operator==(const ::std::wstring& s) const;

				/// Behavior of Iterator
				item_proxy & operator=(const item_proxy&);

				/// Behavior of Iterator
				item_proxy & operator++();

				/// Behavior of Iterator
				item_proxy	operator++(int);

				/// Behavior of Iterator
				item_proxy& operator*();

				/// Behavior of Iterator
				const item_proxy& operator*() const;

				/// Behavior of Iterator
				item_proxy* operator->();

				/// Behavior of Iterator
				const item_proxy* operator->() const;

				/// Behavior of Iterator
				bool operator==(const item_proxy&) const;

				/// Behavior of Iterator
				bool operator!=(const item_proxy&) const;

				//Undocumented method
				essence_t * _m_ess() const;
			private:
				std::vector<cell> & _m_cells() const;
				nana::any         * _m_value(bool alloc_if_empty);
				const nana::any   * _m_value() const;
			private:
				essence_t * ess_;
				category_t*	cat_{nullptr};

				index_pair	pos_; //Position of an item, it never represents a category when item proxy is available.
			};

			class cat_proxy
				: public std::iterator < std::input_iterator_tag, cat_proxy >
			{
			public:
				using inline_notifier_interface = drawerbase::listbox::inline_notifier_interface;

				cat_proxy() = default;
				cat_proxy(essence_t*, size_type pos);
				cat_proxy(essence_t*, category_t*);

				/// Append an item at abs end of the category, set_value determines whether assign T object to the value of item.
				template<typename T>
				item_proxy append(T&& t, bool set_value = false)
				{
					oresolver ores;
					if (set_value)
						ores << t;	//copy it if it is rvalue and set_value is true.
					else
						ores << std::forward<T>(t);

					_m_append(ores.move_cells());

					item_proxy iter{ ess_, index_pair(pos_, size() - 1) };
					if (set_value)
						iter.value(std::forward<T>(t));

					_m_update();

					return iter;
				}

				/// Appends one item at the end of this category with the specifies text in the column fields
				void append(std::initializer_list<std::string> texts_utf8);
				void append(std::initializer_list<std::wstring> texts);

				size_type columns() const;

				cat_proxy& text(std::string);
				cat_proxy& text(std::wstring);
				std::string text() const;

				cat_proxy & select(bool);
				bool selected() const;

				/// Behavior of a container
				void push_back(std::string text_utf8);

				item_proxy begin() const;
				item_proxy end() const;
				item_proxy cbegin() const;
				item_proxy cend() const;

				item_proxy at(size_type pos_abs) const;
				item_proxy back() const;

				/// Returns the absolute index of a item by its display pos, the index of the item isn't changed after sorting.
				/// convert from display order to absolute (find the real item in that display pos) but without check from current active sorting, in fact using just the last sorting !!!
				size_type index_by_display_order(size_type disp_order) const;
				
          		/// find display order for the real item but without check from current active sorting, in fact using just the last sorting !!!
                size_type display_order(size_type pos) const;
				
                /// this cat position
                size_type position() const;

				/// Returns the number of items
				size_type size() const;

				/// Behavior of Iterator
				cat_proxy& operator=(const cat_proxy&);

				/// Behavior of Iterator
				cat_proxy & operator++();

				/// Behavior of Iterator
				cat_proxy	operator++(int);

				/// Behavior of Iterator
				cat_proxy& operator*();

				/// Behavior of Iterator
				const cat_proxy& operator*() const;

				/// Behavior of Iterator
				cat_proxy* operator->();

				/// Behavior of Iterator
				const cat_proxy* operator->() const;

				/// Behavior of Iterator
				bool operator==(const cat_proxy&) const;

				/// Behavior of Iterator
				bool operator!=(const cat_proxy&) const;

				void inline_factory(size_type column, pat::cloneable<pat::abstract_factory<inline_notifier_interface>> factory);
			private:
				void _m_append(std::vector<cell> && cells);
				void _m_cat_by_pos();
				void _m_update();
			private:
				essence_t*	ess_{nullptr};
				category_t*	cat_{nullptr};
				size_type	pos_{0};  ///< Absolute position, not relative to display, and dont change during sort()
			};
		
			struct export_options
			{
				std::string sep = ::std::string {"\t"}, 
							 endl= ::std::string {"\n"};
				bool only_selected_items{true}, 
					 only_checked_items {false},
					 only_visible_columns{true};

				using columns_indexs = std::vector<size_type>;
				columns_indexs columns_order;
			};
		}
	}//end namespace drawerbase

    struct arg_listbox
        : public event_arg
    {
        mutable drawerbase::listbox::item_proxy item;
        bool    selected;

        arg_listbox(const drawerbase::listbox::item_proxy&, bool selected) noexcept;
    };

	/// The event argument type for listbox's category_dbl_click
    struct arg_listbox_category
		: public event_arg
	{
        drawerbase::listbox::cat_proxy category;

		/// Block expension/shrink of category
        void block_category_change() const noexcept;

		/// Determines whether expension/shrink of category is blocked
        bool category_change_blocked() const noexcept;

		arg_listbox_category(const drawerbase::listbox::cat_proxy&) noexcept;
    private:
        mutable bool block_change_;
	};

	namespace drawerbase
	{
		namespace listbox
		{
			struct listbox_events
				: public general_events
			{
				/// An envent occurs when the toggle of a listbox item is checked.
				basic_event<arg_listbox> checked;

				/// An event occurs when a listbox item is clicked.
				basic_event<arg_listbox> selected;

				/// An event occurs when a listbox category is double clicking.
                basic_event<arg_listbox_category> category_dbl_click;
			};

			struct scheme
				: public widget_geometrics
			{
				color_proxy header_bgcolor{static_cast<color_rgb>(0xf1f2f4)};
				color_proxy header_grabbed{ static_cast<color_rgb>(0x8BD6F6)};
				color_proxy header_floated{ static_cast<color_rgb>(0xBABBBC)};
				color_proxy item_selected{ static_cast<color_rgb>(0xD5EFFC) };

                  /// \todo how to implement some geometrical parameters ??
				unsigned max_header_width{ 3000 };  ///< during auto width don't alow more than this           
				unsigned min_header_width{ 20   };  ///< def=20 . non counting suspension_width           
				unsigned suspension_width{ 8    };  ///<  def= . the trigger will set this to the width if ("...")
				unsigned ext_w           { 5    };  ///<  def= 5. Additional or extended with added (before) to the text width to determine the cell width. cell_w = text_w + ext_w +1
				unsigned header_height   { 25   };  ///<  def=25 . header height   header_size
				unsigned text_height     { 14   };  ///< the trigger will set this to the height of the text font
				unsigned item_height_ex  { 6    };  ///< Set !=0 !!!!  def=6. item_height = text_height + item_height_ex
				unsigned item_height     { 24   };  ///<  def=24 . the trigger will set this TO item_height = text_height + item_height_ex
				unsigned header_mouse_spliter_area_before{ 2 }; ///< def=2. But 4 is better... IMO
				unsigned header_mouse_spliter_area_after { 3 }; ///< def=3. But 4 is better... 

				//void debug_print(const std::string &msg);

			};
		}
	}//end namespace drawerbase

/*! \class listbox
\brief A rectangle containing a list of strings from which the user can select. 
This widget contain a list of \a categories, with in turn contain a list of \a items. 
A \a category is a text with can be \a selected, \a checked and \a expanded to show the \a items.
An \a item is formed by \a column-fields, each corresponding to one of the \a headers. 
An \a item can be \a selected and \a checked.
The user can \a drag the header to \a resize it or to \a reorganize it. 
By \a clicking on one header the list get \a reordered, first up, and then down alternatively.

1. The resolver is used to resolute an object of the specified type into (or back from) a listbox item.
3. nana::listbox creates the category 0 by default. 
   This is an special category, becouse it is invisible, while the associated items are visible. 
   The optional, user-created categories begin at index 1 and are visibles.
   The member functions without the categ parameter operate the items that belong to category 0.
4. A sort compare is used for sorting the items. It is a strict weak ordering comparer that must meet the requirement:
		Irreflexivity (comp(x, x) returns false) 
	and 
		Antisymmetry(comp(a, b) != comp(b, a) returns true)
	A simple example.
		bool sort_compare( const std::string& s1, nana::any*, 
						   const std::string& s2, nana::any*, bool reverse)
		{
			return (reverse ? s1 > s2 : s1 < s2);
		}
		listbox.set_sort_compare(0, sort_compare);
	The listbox supports attaching a customer's object for each item, therefore the items can be 
	sorted by comparing these customer's object.
		bool sort_compare( const std::string&, nana::any* o1, 
						   const std::string&, nana::any* o2, bool reverse)
		{
			if(o1 && o2) 	//some items may not attach a customer object.
			{
				int * i1 = any_cast<int>(*o1);
				int * i2 = any_cast<int>(*o2);
				return (i1 && i2 && (reverse ? *i1 > *i2 : *i1 < *i2));
 					  // ^ some types may not be int.
			}
			return false;
		}
		listbox.anyobj(0, 0, 10); //the type of customer's object is int.
		listbox.anyobj(0, 0, 20);
5. listbox is a widget_object, with template parameters drawerbase::listbox::trigger and drawerbase::listbox::scheme 
amon others.
That means that listbox have a member trigger_ constructed first and accecible with get_drawer_trigger() and
a member (unique pointer to) scheme_ accesible with scheme_type& scheme() created in the constructor 
with API::dev::make_scheme<Scheme>() which call API::detail::make_scheme(::nana::detail::scheme_factory<Scheme>())
which call restrict::bedrock.make_scheme(static_cast<::nana::detail::scheme_factory_base&&>(factory));
which call pi_data_->scheme.create(std::move(factory));
which call factory.create(scheme_template(std::move(factory)));
which call (new Scheme(static_cast<Scheme&>(other)));
and which in create is setted with: API::dev::set_scheme(handle_, scheme_.get()); which save the scheme pointer in 
the nana::detail::basic_window member pointer scheme
\todo doc: actualize this example listbox.at(0)...
\see nana::drawerbase::listbox::cat_proxy
\see nana::drawerbase::listbox::item_proxy
\example listbox_Resolver.cpp
*/
	class listbox
		:	public widget_object<category::widget_tag, drawerbase::listbox::trigger, drawerbase::listbox::listbox_events, drawerbase::listbox::scheme>,
			public concepts::any_objective<drawerbase::listbox::size_type, 2>
	{
	public:
		using size_type		= drawerbase::listbox::size_type;
		using index_pair	= drawerbase::listbox::index_pair;
		using cat_proxy		= drawerbase::listbox::cat_proxy;
		using item_proxy	= drawerbase::listbox::item_proxy;
		using selection = drawerbase::listbox::selection;    ///<A container type for items.
		using iresolver = drawerbase::listbox::iresolver;
		using oresolver = drawerbase::listbox::oresolver;
		using cell      = drawerbase::listbox::cell;
		using export_options= drawerbase::listbox::export_options;
		using columns_indexs= drawerbase::listbox::size_type;
		using inline_notifier_interface = drawerbase::listbox::inline_notifier_interface;
	public:
		listbox() = default;
		listbox(window, bool visible);
		listbox(window, const rectangle& = {}, bool visible = true);

	//Element access

		/// Returns the category at specified location pos, with bounds checking.
		cat_proxy at(size_type pos);
		const cat_proxy at(size_type pos) const;

		/// Returns the item at specified absolute position
		item_proxy at(const index_pair& abs_pos);
		const item_proxy at(const index_pair &abs_pos) const;

		/// Returns the category at specified location pos, no bounds checking is performed.
		cat_proxy operator[](size_type pos);
		const cat_proxy operator[](size_type pos) const;

		/// Returns the item at specified absolute position, no bounds checking is performed.
		item_proxy operator[](const index_pair& abs_pos);
		const item_proxy operator[](const index_pair &abs_pos) const;

	//Associative element access

		/// Returns a proxy to the category of the key or create a new one in the right order
		/**
		* @param key The key of category to find
		* @return A category proxy
		*/
		template<typename Key>
		cat_proxy assoc(Key&& key)
		{
			using key_type = typename ::nana::detail::type_escape<const typename std::decay<Key>::type>::type;

			auto p = std::make_shared<nana::key<key_type, std::less<key_type>>>(std::forward<Key>(key));
			return cat_proxy(&_m_ess(), _m_assoc(p, true));
		}

		/// Returns a proxy to the category of the key or create a new one in the right order
		/**
		* @param key The key of category to find
		* @return A category proxy
		*/
		template<typename Key>
		cat_proxy assoc_at(Key&& key)
		{
			using key_type = typename ::nana::detail::type_escape<const typename std::decay<Key>::type>::type;

			auto p = std::make_shared<nana::key<key_type, std::less<key_type>>>(std::forward<Key>(key));

			auto categ = _m_assoc(p, false);
			if (nullptr == categ)
				throw std::out_of_range("listbox: invalid key.");

			return cat_proxy(&_m_ess(), categ);
		}

		/// Removes a category which is associated with the specified key
		/**
		* @param key The key of category to remove
		*/
		template<typename Key>
		void assoc_erase(Key&& key)
		{
			using key_type = typename ::nana::detail::type_escape<const typename std::decay<Key>::type>::type;

			::nana::key<key_type, std::less<key_type>> wrap(key);
			_m_erase_key(&wrap);
		}

		bool assoc_ordered(bool);


		void auto_draw(bool);                                ///< Set state: Redraw automatically after an operation

		template<typename Function>
		void avoid_drawing(Function fn)
		{
			this->auto_draw(false);
			try
			{
				fn();
			}
			catch (...)
			{
				this->auto_draw(true);
				throw;
			}
			this->auto_draw(true);
		}

		/// Scrolls the view to the first or last item of a specified category
		void scroll(bool to_bottom, size_type cat_pos = ::nana::npos);
		void scroll(bool to_bottom, const index_pair& pos);

        /// Appends a new column with a header text and the specified width at the end, and return it position
		size_type append_header(std::string text_utf8, unsigned width = 120);
		size_type append_header(std::wstring text, unsigned width = 120);
        listbox& header_width(size_type position, unsigned pixels);
		unsigned header_width(size_type position) const;
        unsigned auto_width(size_type position, unsigned max=3000);


		cat_proxy append(std::string);          ///< Appends a new category to the end
		cat_proxy append(std::wstring);			///< Appends a new category to the end
		void append(std::initializer_list<std::string>); ///< Appends categories to the end
		void append(std::initializer_list<std::wstring>); ///< Appends categories to the end

		cat_proxy insert(cat_proxy, ::std::string);
		cat_proxy insert(cat_proxy, ::std::wstring);

		/// Inserts an item before a specified position
		/**
		 * @param abs_pos The absolute position before which an item will be inserted.
		 * @param text Text of the first column, in UTF-8 encoded.
		 */
		void insert_item(const index_pair& abs_pos, ::std::string text);

		/// Inserts an item before a specified position
		/**
		 * @param abs_pos The absolute position before which an item will be inserted.
		 * @param text Text of the first column.
		 */
		void insert_item(const index_pair& abs_pos, ::std::wstring text);



		/// Returns an index of item which contains the specified point.
		index_pair cast(const point & pos) const;

		/// Returns the column which contains the specified point.
        columns_indexs column_from_pos(const point & pos);

		void checkable(bool);
		selection checked() const;                         ///<Returns the items which are checked.                       

		void clear(size_type cat);                         ///<Removes all the items from the specified category
		void clear();                                      ///<Removes all the items from all categories
		void erase(size_type cat);                         ///<Erases a category
		void erase();                                      ///<Erases all categories.
		item_proxy erase(item_proxy);

		bool sortable() const;
		void sortable(bool enable);
		
		///Sets a strict weak ordering comparer for a column
		void set_sort_compare(size_type col, std::function<bool(const std::string&, nana::any*,
				                                        const std::string&, nana::any*, bool reverse)> strick_ordering);

		/// sort() and ivalidate any existing reference from display position to absolute item, that is: after sort() display offset point to different items
        void sort_col(size_type col, bool reverse = false);
		size_type sort_col() const;

        /// potencially ivalidate any existing reference from display position to absolute item, that is: after sort() display offset point to different items
        void unsort();
		bool freeze_sort(bool freeze);

		selection selected() const;                         ///<Get the absolute indexs of all the selected items
                                    
		void show_header(bool);
		bool visible_header() const;
		void move_select(bool upwards);  ///<Selects an item besides the current selected item in the display.

		size_type size_categ() const;                   ///<Get the number of categories
		size_type size_item() const;                    ///<The number of items in the default category
		size_type size_item(size_type cat) const;       ///<The number of items in category "cat"

		void enable_single(bool for_selection, bool category_limited);
		void disable_single(bool for_selection);
        export_options& def_export_options();
	private:
		drawerbase::listbox::essence_t & _m_ess() const;
		nana::any* _m_anyobj(size_type cat, size_type index, bool allocate_if_empty) const;
		drawerbase::listbox::category_t* _m_assoc(std::shared_ptr<nana::detail::key_interface>, bool create_if_not_exists);
		void _m_erase_key(nana::detail::key_interface*);
	};
}//end namespace nana

#include <nana/pop_ignore_diagnostic>
#endif
