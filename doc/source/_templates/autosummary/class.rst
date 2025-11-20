{{ fullname | escape | underline}}

.. currentmodule:: {{ module }}

{# Build list of overloaded methods to exclude from autodoc #}
{%- set overloaded_methods = [] %}
{%- if methods %}
{%- for item in methods %}
{%- if get_overloads(fullname ~ '.' ~ item) %}
{%- set _ = overloaded_methods.append(item) %}
{%- endif %}
{%- endfor %}
{%- endif %}

.. autoclass:: {{ fullname }}
   :members:
   :member-order: bysource
   :exclude-members: __delattr__, __dir__, __eq__, __format__, __ge__, __getattribute__, __getstate__, __gt__, __hash__, __init_subclass__, __le__, __lt__, __ne__, __new__, __reduce__, __reduce_ex__, __repr__, __setattr__, __sizeof__, __str__, __subclasshook__, __weakref__, __dict__, __annotations__, __doc__, __module__{% if is_enum_class(fullname) %}, __init__{% endif %}{% if attributes %}, {{ attributes|join(', ') }}{% endif %}{% if overloaded_methods %}, {{ overloaded_methods|join(', ') }}{% endif %}

   {% block methods %}
   {# Skip methods table for Enum classes - they should only show enum values #}
   {% if methods and not is_enum_class(fullname) %}
   .. rubric:: {{ _('Methods') }}

   .. autosummary::
   {% for item in methods %}
      {%- if item not in ['__delattr__', '__dir__', '__eq__', '__format__', '__ge__', '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init_subclass__', '__le__', '__lt__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__'] %}
      {%- set method_overloads = get_overloads(fullname ~ '.' ~ item) %}
      {%- if method_overloads %}
      ~{{ name }}.{{ item }}
      {%- else %}
      ~{{ name }}.{{ item }}
      {%- endif %}
      {%- endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block attributes %}
   {# Skip attributes table for Enum classes - autodoc handles them properly #}
   {% if attributes and not is_enum_class(fullname) %}
   .. rubric:: {{ _('Attributes') }}

   .. list-table::
      :widths: 25 20 55
      :header-rows: 1

      * - Name
        - Type
        - Description
      {% for item in attributes %}
      {%- if item not in ['__annotations__', '__doc__', '__module__', '__dict__', '__weakref__', '__members__', '__name__', '__qualname__'] %}
      {%- set item_type = attr_type(fullname, item) %}
      {%- set nested_doc = get_nested_class_doc(fullname, item) %}
      * - **{{ item }}**
        - {% if nested_doc %}:ref:`{{ item_type }} <{{ fullname }}.{{ item_type }}>`{% else %}{{ make_type_xref(item_type) }}{% endif %}

        - {{ attr_doc(fullname, item) }}
      {%- endif %}
      {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block nested_classes %}
   {# Document nested classes used by attributes #}
   {% if attributes %}
   {%- set documented_nested = [] %}
   {% for item in attributes %}
   {%- if item not in ['__annotations__', '__doc__', '__module__', '__dict__', '__weakref__', '__members__', '__name__', '__qualname__'] %}
   {%- set item_type = attr_type(fullname, item) %}
   {%- set nested_doc = get_nested_class_doc(fullname, item) %}
   {%- set nested_attrs = get_nested_class_attrs(fullname, item) %}
   {%- if nested_doc and item_type not in documented_nested %}
   {%- set _ = documented_nested.append(item_type) %}

   .. _{{ fullname }}.{{ item_type }}:

   **{{ item_type }}**

{{ nested_doc | napoleon | indent(3, first=True) }}

   {%- if nested_attrs %}

   .. list-table::
      :widths: 30 20 50
      :header-rows: 1

      * - Attribute
        - Type
        - Description
      {%- for nested_attr in nested_attrs %}
      * - ``{{ nested_attr.name }}``
        - ``{{ nested_attr.type }}``
        - {{ nested_attr.doc }}
      {%- endfor %}
   {%- endif %}
   {%- endif %}
   {%- endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block overloaded_methods %}
   {# Document overloaded methods explicitly - INSIDE autodoc directive for proper styling #}
   {% if methods %}
   {% for item in methods %}
   {%- set method_overloads = get_overloads(fullname ~ '.' ~ item) %}
   {%- if method_overloads %}

   {% for overload in method_overloads %}
   .. py:method:: {{ item }}({% for arg_name, arg_type in overload.args %}{% if arg_name != 'self' %}{{ arg_name }}: {{ arg_type }}{% if not loop.last %}, {% endif %}{% endif %}{% endfor %}) -> {{ overload.return_type }}
      :noindex:

      {% if overload.docstring %}
{{ overload.docstring | napoleon | indent(6, first=True) }}
      {% endif %}

   {% endfor %}
   {%- endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}
