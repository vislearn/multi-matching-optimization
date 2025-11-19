{{ fullname | escape | underline}}

.. currentmodule:: {{ module }}

{% set overloads = get_overloads(fullname) %}
{% if overloads %}
{# Function has overloads - document each signature with its own docstring #}
{% for overload in overloads %}
.. py:function:: {{ objname }}({% for arg_name, arg_type in overload.args %}{{ arg_name }}: {{ arg_type }}{% if not loop.last %}, {% endif %}{% endfor %}) -> {{ overload.return_type }}
{% if not loop.first %}   :noindex:
{% endif %}
{% if overload.docstring %}

{{ overload.docstring | napoleon | indent(3, first=True) }}
{% endif %}

{% endfor %}

{% else %}
{# Regular function - use standard autodoc #}
.. autofunction:: {{ objname }}
{% endif %}
