/**
 * @file
 * Add some extra reactivity to the chunks field widget.
 */

(function($){

  'use strict';

  // Remove active state from element by briefly removing it from the DOM.
  var removeActiveState = function(el) {
    var p, s;
    p = el.parentNode;
    s = el.nextSibling;

    p.removeChild(el);
    p.insertBefore(el, s);
  };

  Drupal.behaviors.chunksWidget = {
    attach: function(context, settings) {
      // Do some stuff
      $('.chunks-field', context).each(function() {
        var fieldName, langcode;

        fieldName = $(this).attr('field_name').replace(/_/g, '-');
        langcode = $(this).attr('langcode');

        $('.item', $(this)).each(function(delta, element) {
          var classPrepend = '.' + fieldName + '-' + delta + '-';

          // Show the preview for a Chunk when the user presses Preview.
          $(classPrepend + 'preview-button', element).bind('mousedown.chunkPreview', function(e) {
            $(classPrepend + 'configuration').hide();
            $(classPrepend + 'edit-button').show();
            $(this).hide();
            removeActiveState(this);
          });

          // Automatically display the content entry form when the user selects
          // a chunk type.
          $(classPrepend + 'edit-button', element).bind('mousedown.chunkEdit', function(e) {
            $(classPrepend + 'configuration').show();
            $(classPrepend + 'preview-button').show();
            $(this).hide();
            removeActiveState(this);
          });
        });

      });
    }
  };

})(jQuery);

