/**
 * @file
 *   JS behavior for Quote chunk type.
 */

(function($) {

  'use strict';

  // Instantiates the Pen editor.
  Drupal.behaviors.chunksQuoteInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(index, element) {
        var fieldName, langcode, fieldSettings, initializePen;

        fieldName = $(this).attr('field_name');
        langcode = $(this).attr('langcode');
        fieldSettings = settings.chunks[fieldName];

        // Don't do anything if the Pen library isn't loaded.
        if (typeof Pen !== 'function') {
          return;
        }

        // Only initialize Pen if it's needed for a chunk instance.
        for (var instanceName in fieldSettings.instances) {
          if (fieldSettings.instances[instanceName].type === 'quote' && fieldSettings.instances[instanceName].settings.edit_in_place) {
            initializePen = true;
            break;
          }
        }

        // Do not continue if there were no quote chunk instances with the
        // edit_in_place setting.
        if (!initializePen) {
          return;
        }

        // Sometimes the pen-quote-menu doesn't like to go away. Let's force it.
        $('.pen-quote-quote-menu, .pen-quote-attribution-menu').hide();
        $(':input', context).bind('mousedown.hidePenMenu', function() {
          $('.pen-quote-quote-menu, .pen-quote-attribution-menu').hide();
        });

        // Initialize editor for the paragraph in this Quote chunk
        $('.quote-chunk > p[contenteditable], .quote-chunk cite[contenteditable]').once(function() {
          var editor, editorConfig, configWrapper, hiddenField;

          editorConfig = {
            class: this.nodeName === 'P' ? 'pen-quote-quote' : 'pen-quote-attribution',
            editor: this,
            list: ['bold', 'italic', 'underline', 'createlink'],
            stay: false
          };

          editor = new Pen(editorConfig);

          configWrapper = $(this).parents('.quote-chunk-configuration');

          if (this.nodeName === 'P') {
            // Hide the regular textarea.
            hiddenField = configWrapper.find('[name$="[quote]"]');
            hiddenField.parent().parent().hide();
          }
          else {
            // Hide the regular textfield.
            hiddenField = configWrapper.find('[name$="[attribution]"]');
            hiddenField.parent().hide();
          }

          // Update hidden field on keyup, blur, paste, and cut events.
          $(this).bind('keyup.chunksQuoteInPlace blur.chunksQuoteInPlace paste.chunksQuoteInPlace cut.chunksQuoteInPlace', function(e) {
            var editableElement = this;

            // Each time the configuration could have changed, copy the data
            // back into the form item so it gets saved when the user submits
            // the form. setTimeout is necessary because some events fire
            // before the markup has changed.
            setTimeout(function() {
              hiddenField.val($(editableElement).html());
            }, 0);
          });

          // Prevent <div></div> tags from being added when user presses
          // "Enter".
          $(this).bind('keydown.chunksQuoteNoDiv', function(e) {
            if (e.keyCode === 13) {
              document.execCommand('insertHTML', false, '<br><br>');
              return false;
            }
          });
        });

        // Trigger blur event on all contenteditable quote chunks when the
        // pen toolbar is clicked so we save configuration correctly.
        $('.pen-quote-quote-menu').bind('click.chunksQuoteBlur', function() {
          $('.quote-chunk > p[contenteditable]').trigger('blur.chunksQuoteInPlace');
        });
        $('.pen-quote-attribution-menu').bind('click.chunksQuoteBlur', function() {
          $('.quote-chunk cite[contenteditable]').trigger('blur.chunksQuoteInPlace');
        });
      });
    },
  };

  Drupal.behaviors.chunksQuoteCallbacks = {
    attach: function(context, settings) {

      // No need to do anything if we've already created the callback.
      if (typeof Drupal.settings.chunks.callbacks.quote.restoreConfig !== 'undefined') {
        return;
      }

      // Implements the restoreConfig callback to restore saved configuration.
      Drupal.settings.chunks.callbacks.quote.restoreConfig = function(fieldName, langcode, delta) {
        var chunkInstance;

        // Retrieve the chunk instance.
        chunkInstance = Drupal.chunks.fields[fieldName].chunks[delta].chunkInstance;

        // Only do anything if this instance is set to be edited in-place.
        if (Drupal.settings.chunks[fieldName].instances[chunkInstance].settings.edit_in_place) {
          var classFieldName, quoteConfig, configuration, configState, inPlaceEditor;

          classFieldName = fieldName.replace(/_/g, '-');
          quoteConfig = $('#' + fieldName + '-' + delta + '-chunk .quote-chunk-configuration');

          // Make a copy of the configuration and add the edit_in_place
          // property so we can render a contenteditable quote chunk
          // without changing the configuration settings for the chunk.
          configState = Drupal.settings.chunks[fieldName].chunks[delta].configuration[chunkInstance];
          configuration = {};
          for (var prop in configState) {
            configuration[prop] = configState[prop];
          }
          configuration.edit_in_place = true;

          // Generate new markup.
          inPlaceEditor = Drupal.theme.prototype.chunk__quote(configuration, fieldName, langcode, delta);

          // Insert that new markup into the editor.
          quoteConfig.find('.quote-chunk').remove();
          quoteConfig.append(inPlaceEditor);
          Drupal.behaviors.chunksQuoteInPlace.attach(quoteConfig.parents('.field-type-chunks'), Drupal.settings);
        }
      };
    },
  };

  // Provide a client-side theme implementation for text chunks.
  Drupal.theme.prototype.chunk__quote = function(config, fieldName, langcode, delta) {
    var output, contentEditable;

    // Add contenteditable attribute if we're editing this chunk in-place.
    if (typeof config.edit_in_place !== 'undefined') {
      contentEditable = config.edit_in_place ? ' contenteditable' : '';
    }
    else {
      contentEditable = '';
    }

    // Add the cite_attribute property if the attribution_location property
    // exists.
    if (config.attribution_location !== '') {
      config.attribution_location = Drupal.checkPlain(config.attribution_location);
      config.cite_attribute = config.attribution_location !== '' ? ' cite="' + config.attribution_location + '"' : '';
    }
    else {
      config.cite_attribute = '';
    }

    output = '<blockquote class="chunk quote-chunk"' + config.cite_attribute + '>';
    output += '<p' + contentEditable + '>' + config.quote + '</p>';

    if (config.attribution !== '') {
      output += '<footer><cite' + contentEditable + '>' + config.attribution + '</cite></footer>';
    }

    output += '</blockquote>';

    return output;
  };

})(jQuery);
