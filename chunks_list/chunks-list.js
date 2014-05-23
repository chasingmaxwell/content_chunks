/**
 * @file Provide some additional interactivity for the list chunk type.
 */

(function($) {

  // Handle in-place editing for chunks.
  Drupal.behaviors.chunksListInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function() {

        fieldName = $(this).attr('field_name');

        if (settings.chunks[fieldName].types.list.instance_type_settings.edit_in_place && typeof Pen === 'function') {
          // Sometimes the pen-menu doesn't like to go away. Let's force it.
          $('.pen-menu').hide();
          $(':input', context).bind('mousedown.hidePenMenu', function() {
            $('.pen-menu').hide();
          });

          // Perform actions on each list chunk with the contenteditable
          // attribute.
          $('.list-chunk[contenteditable]', context).once(function() {

            // Initialize pen.
            var editor = new Pen({
              editor: this,
                list: ['bold', 'italic', 'underline', 'createlink'],
                stay: false
            });

            // Hide regular textarea.
            $(this).prev().hide();

            // Update textarea on keyup and blur.
            $(this).bind('keyup.chunksListInPlace blur.chunksListInPlace', function(e) {
              var list_items = [];
              $(this).find('li').each(function() {
                list_items.push($(this).html().replace(/\r?\n|\r/g, ''));
              });
              list_items = list_items.join("\n");
              $(this).parent().find('textarea').val(list_items);
            });
          });

          // Trigger blur event on all contenteditable list chunks when the pen
          // toolbar is clicked so we save configuration correctly.
          $('.pen-menu').bind('click.chunksListBlur', function() {
            $('.list-chunk[contenteditable]').trigger('blur.chunksListInPlace');
          });
        }
      });
    }
  };


  // Provide a client-side theme implementation for list chunks.
  Drupal.theme.prototype.chunk__list = function(configuration) {
    var list, output, list_item;

    // Break configuration.list into an array.
    list = configuration.list.split(/\n/);

    // Add contenteditable attribute if we're editing this chunk in-place.
    if (typeof configuration.edit_in_place !== 'undefined') {
      contentEditable = configuration.edit_in_place ? ' contenteditable' : '';
    }
    else {
      contentEditable = '';
    }

    output = '<' + configuration.style + ' class="chunk list-chunk"' + contentEditable + '>';

    for (var key in list) {

      // If the format is plain text, run the text through chunkPlain
      if (Drupal.settings.chunks[fieldName].types.list.instance_type_settings.format === 'plain_text') {
        list_item = Drupal.checkPlain(list[key]);
      }
      // Otherwise don't do anything at all.
      else {
        list_item = list[key];
      }

      // Do not display empty strings.
      if (list_item === '') {
        continue;
      }

      // Add the list item to output.
      output += '<li>' + list_item + '</li>';
    }

    output += '</' + configuration.style + '>';

    return output;
  };

  Drupal.behaviors.chunksListCallbacks = {
    attach: function(context, settings) {

      // Implements the saveConfig callback to save list items.
      if (typeof settings.chunks.callbacks.list.restoreConfig === 'undefined') {
        settings.chunks.callbacks.list.restoreConfig = function(fieldName, langcode, delta) {
          var classFieldName, preview, listConfig;
          classFieldName = fieldName.replace(/_/g, '-');
          Drupal.settings.chunks[fieldName].chunks[delta].configuration.list.edit_in_place = true;
          preview = Drupal.theme.prototype.chunk__list(Drupal.settings.chunks[fieldName].chunks[delta].configuration.list, fieldName, langcode, delta);
          listConfig = $('#' + fieldName + '-' + delta + '-chunk .list-chunk-configuration');
          listConfig.find('.list-chunk').remove();
          listConfig[0].innerHTML += preview;
          Drupal.attachBehaviors(listConfig.parents('.field-type-chunks'));
        };
      }
    },
  };
})(jQuery);
